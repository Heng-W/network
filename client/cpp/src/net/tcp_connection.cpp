
#include "tcp_connection.h"

#include <errno.h>
#include "../util/logger.h"
#include "socket.h"

namespace net
{

constexpr int kMaxReadSize = 65536;

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    LOG(TRACE) << conn->localAddr().toIpPort() << " -> "
               << conn->peerAddr().toIpPort() << " is "
               << (conn->connected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf, Timestamp)
{
    buf->retrieveAll();
}

TcpConnection::TcpConnection(int sockfd, const InetAddress& peerAddr)
    : state_(kConnecting),
      sockfd_(sockfd),
      localAddr_(sockets::getLocalAddr(sockfd)),
      peerAddr_(peerAddr)
{
    LOG(DEBUG) << "TcpConnection::constructor[" << "] at " << this << " fd=" << sockfd;
}

TcpConnection::~TcpConnection()
{
    LOG(DEBUG) << "TcpConnection::destructor[" << "] at " << this
               << " fd=" << sockfd_
               << " state=" << stateToString();
    assert(state_ == kDisconnected);
    sockets::close(sockfd_);
}

void TcpConnection::send(const void* data, int len)
{
    if (state_ == kConnected)
    {
        Buffer buf(len, 0);
        buf.append(data, len);
        send(std::move(buf));
    }
}

void TcpConnection::send(const std::string& message)
{
    send(message.data(), message.size());
}

void TcpConnection::send(Buffer&& buf)
{
    if (state_ == kConnected)
    {
        buffersToSend_.put(std::move(buf));
    }
}

bool TcpConnection::sendInThread(const void* data, int len)
{
    int n = sockets::write(sockfd_, data, len);
    if (n < 0)
    {
        SYSLOG(ERROR) << "TcpConnection::sendInThread";
        int err = sockets::getSocketError(sockfd_);
        LOG(ERROR) << "socket error ["
                   << "] - SO_ERROR = " << err << " " << strerror(err);
        return false;
    }
    assert(n == len);
    if (writeCompleteCallback_ && buffersToSend_.size() == 0)
    {
        writeCompleteCallback_(shared_from_this());
    }
    return true;
}

void TcpConnection::doSendEvent()
{
    while (state_ == kConnected)
    {
        Buffer buf = buffersToSend_.get();
        LOG(TRACE) << stateToString();
        if (state_ != kConnected || !sendInThread(buf.peek(), buf.readableBytes()))
        {
            break;
        }
    }
    setState(kDisconnecting);
    sockets::shutdownReadWrite(sockfd_);
}


void TcpConnection::doRecvEvent()
{
    while (state_ == kConnected)
    {
        inputBuffer_.ensureWritableBytes(kMaxReadSize);
        int n = sockets::read(sockfd_, inputBuffer_.beginWrite(), kMaxReadSize);
        LOG(TRACE) << stateToString();
        if (state_ != kConnected) break;

        if (n > 0)
        {
            inputBuffer_.hasWritten(n);
            messageCallback_(shared_from_this(), &inputBuffer_, Timestamp::now());
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            SYSLOG(ERROR) << "TcpConnection::doRecvEvent";
            int err = sockets::getSocketError(sockfd_);
            LOG(ERROR) << "socket error ["
                       << "] - SO_ERROR = " << err << " " << strerror(err);
            break;
        }
    }
    shutdown();
}

void TcpConnection::connectEstablished()
{
    assert(state_ == kConnecting);
    setState(kConnected);

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    LOG(TRACE) << "fd = " << sockfd_ << " state = " << stateToString();
    assert(state_ == kDisconnecting);
    setState(kDisconnected);

    connectionCallback_(shared_from_this());
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        buffersToSend_.put(Buffer(0));
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    sockets::setTcpNoDelay(sockfd_, on);
}

const char* TcpConnection::stateToString() const
{
    State state = state_;
    switch (state)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

} // namespace net
