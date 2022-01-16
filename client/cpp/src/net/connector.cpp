
#include "connector.h"

#include <assert.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/eventfd.h>
#include <unistd.h>
#endif
#include "../util/logger.h"
#include "socket.h"

namespace net
{

Connector::Connector(const InetAddress& serverAddr)
    : serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs),
      fixedRetryDelay_(false)
{
    createWakeupfd();
    LOG(DEBUG) << "constructor[" << this << "]";
}

Connector::~Connector()
{
    LOG(DEBUG) << "destructor[" << this << "]";
#ifdef _WIN32
    sockets::close(wakeupSendFd_);
    sockets::close(wakeupRecvFd_);
#else
    ::close(wakeupFd_);
#endif
}

void Connector::start()
{
    assert(state_ == kDisconnected);
    connect_ = true;
    doConnectEvent();
}

void Connector::stop()
{
    connect_ = false;
    wakeup();
}

void Connector::doConnectEvent()
{
    while (true)
    {
        int sockfd = tryConnect();
        if (sockfd < 0)
        {
            if (!connect_) return;
            LOG(INFO) << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
                      << " in " << retryDelayMs_ << " milliseconds. ";

            struct timeval timeout;
            timeout.tv_sec = retryDelayMs_ / 1000;
            timeout.tv_usec = (retryDelayMs_ - timeout.tv_sec * 1000) * 1000;

            fd_set readfds;
            FD_ZERO(&readfds);
#ifdef _WIN32
            FD_SET(wakeupRecvFd_, &readfds);
#else
            FD_SET(wakeupFd_, &readfds);
#endif

            if (select(FD_SETSIZE, &readfds, nullptr, nullptr, &timeout) > 0)
            {
                handleRead();
                if (!connect_) return;
            }

            if (!fixedRetryDelay_)
            {
                retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
            }
        }
        else
        {
            LOG(TRACE) << "connect success";
            sockets::setNonBlock(sockfd, false);
            newConnectionCallback_(sockfd);
            return;
        }
    }
}

int Connector::tryConnect()
{
    int sockfd = sockets::createTcpNonBlock();
    int ret = sockets::connect(sockfd, serverAddr_);

#ifdef _WIN32
    int savedErrno = (ret == 0) ? 0 : ::WSAGetLastError();
    if (savedErrno == WSAEWOULDBLOCK)
    {
        return connecting(sockfd);
    }

#else
    int savedErrno = (ret == 0) ? 0 : errno;
    SYSLOG_IF(ERROR, savedErrno != 0) << "connect";

    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            return connecting(sockfd);
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            break;
        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            SYSLOG(ERROR) << "connect error in Connector::startInLoop " << savedErrno;
            connect_ = false;
            break;
        default:
            SYSLOG(ERROR) << "Unexpected error in Connector::startInLoop " << savedErrno;
            connect_ = false;
            break;
    }
#endif
    sockets::close(sockfd);
    setState(kDisconnected);
    return -1;
}

int Connector::connecting(int sockfd)
{
    setState(kConnecting);

    fd_set readfds;
    FD_ZERO(&readfds);
#ifdef _WIN32
    FD_SET(wakeupRecvFd_, &readfds);
#else
    FD_SET(wakeupFd_, &readfds);
#endif

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);

    // 超时时间
    struct timeval timeout;
    timeout.tv_sec  = 5;
    timeout.tv_usec = 0;

    LOG(TRACE) << "connect...";

    if (select(FD_SETSIZE, &readfds, &writefds, nullptr, &timeout) > 0)
    {
#ifdef _WIN32
        if (FD_ISSET(wakeupRecvFd_, &readfds))
#else
        if (FD_ISSET(wakeupFd_, &readfds))
#endif
        {
            if (!connect_) return -1;
        }
        if (FD_ISSET(sockfd, &writefds))
        {
            int err = sockets::getSocketError(sockfd);

            if (err)
            {
                LOG(WARN) << "Connector::handleWrite - SO_ERROR = "
                          << err << " " << strerror(err);

            }
            else if (sockets::isSelfConnect(sockfd))
            {
                LOG(WARN) << "Connector::handleWrite - Self connect";
            }
            else
            {
                setState(kConnected);
                return sockfd;
            }
        }
    }
    sockets::close(sockfd);
    setState(kDisconnected);
    return -1;
}


void Connector::createWakeupfd()
{
#ifdef _WIN32
    // Windows上用一对socket来模拟 eventfd
    Socket wakeupSocket(sockets::createTcp());
    wakeupSocket.setReuseAddr(true);
    wakeupSocket.bind(InetAddress(0, true)); // loop back and random port
    wakeupSocket.listen();

    InetAddress serverAddr = sockets::getLocalAddr(wakeupSocket.fd());
    LOG(DEBUG) << "used port of wakeupfd: " << serverAddr.toPort();

    wakeupSendFd_ = sockets::createTcp();
    sockets::connect(wakeupSendFd_, serverAddr);
    sockets::setNonBlock(wakeupSendFd_);

    wakeupRecvFd_ = wakeupSocket.accept(); // already has been set nonblock
#else
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0)
    {
        SYSLOG(FATAL) << "Failed in eventfd";
    }
    wakeupFd_ = evfd;
#endif
}

void Connector::wakeup()
{
    uint64_t one = 1;
#ifdef _WIN32
    int n = sockets::write(wakeupSendFd_, &one, sizeof(one));
#else
    int n = ::write(wakeupFd_, &one, sizeof(one));
#endif
    if (n != sizeof(one))
    {
        LOG(ERROR) << "Connector::wakeup() writes " << n << " bytes instead of 8";
    }
}

void Connector::handleRead()
{
    uint64_t one = 1;
#ifdef _WIN32
    int n = sockets::read(wakeupRecvFd_, &one, sizeof(one));
#else
    int n = ::read(wakeupFd_, &one, sizeof(one));
#endif
    if (n != sizeof(one))
    {
        LOG(ERROR) << "handleRead reads " << n << " bytes instead of 8";
    }
}

} // namespace net

