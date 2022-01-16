
#include "socket.h"

#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <WinSock2.h>
#ifdef ERROR
#undef ERROR
#endif
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif // _MSC_VER
#else
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // _WIN32
#include "../util/logger.h"
#include "inet_address.h"


namespace net
{

namespace sockets
{

#ifdef _WIN32

struct NetworkInitializer
{
    NetworkInitializer();
    ~NetworkInitializer();
};

NetworkInitializer::NetworkInitializer()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    ::WSAStartup(wVersionRequested, &wsaData);
}

NetworkInitializer::~NetworkInitializer()
{
    ::WSACleanup();
}

NetworkInitializer g_windowsNetworkInitializer;

#endif // _WIN32


static const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
{ return reinterpret_cast<const struct sockaddr*>(addr); }

static struct sockaddr* sockaddr_cast(struct sockaddr_in* addr)
{ return reinterpret_cast<struct sockaddr*>(addr); }

int createTcp()
{
#ifdef _WIN32
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
#endif
    if (sockfd < 0)
    {
        SYSLOG(FATAL) << "sockets::createTcp";
    }
    return sockfd;
}

int createTcpNonBlock()
{
#ifdef _WIN32
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        SYSLOG(FATAL) << "sockets::createTcpNonBlock";
    }
    setNonBlock(sockfd);
#else
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        SYSLOG(FATAL) << "sockets::createTcpNonBlock";
    }
#endif
    return sockfd;
}

int createUdp()
{
#ifdef _WIN32
    int sockfd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    int sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
#endif
    if (sockfd < 0)
    {
        SYSLOG(FATAL) << "sockets::createUdp";
    }
    return sockfd;
}

int createUdpNonBlock()
{
#ifdef _WIN32
    int sockfd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        SYSLOG(FATAL) << "sockets::createUdpNonBlock";
    }
    setNonBlock(sockfd);
#else
    int sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
    if (sockfd < 0)
    {
        SYSLOG(FATAL) << "sockets::createUdpNonblock";
    }
#endif
    return sockfd;
}

void setNonBlock(int sockfd, bool on)
{
#ifdef _WIN32
    unsigned long optval = on ? 1 : 0;
    ::ioctlsocket(sockfd, FIONBIO, &optval);
#else
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    if (flags < 0)
    {
        SYSLOG(FATAL) << "fcntl F_GETFL";
    }
    flags = on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (::fcntl(sockfd, F_SETFL, flags) < 0)
    {
        SYSLOG(FATAL) << "fcntl F_SETFL";
    }
#endif
}

void bind(int sockfd, const InetAddress& addr)
{
    const struct sockaddr_in& sockAddr = addr.getSockAddr();
    int ret = ::bind(sockfd, sockaddr_cast(&sockAddr), static_cast<socklen_t>(sizeof(sockAddr)));
    if (ret < 0)
    {
        SYSLOG(FATAL) << "sockets::bind";
    }
}

void listen(int sockfd)
{
    if (::listen(sockfd, SOMAXCONN) < 0)
    {
        SYSLOG(FATAL) << "sockets::listen";
    }
}

int accept(int sockfd, InetAddress* peerAddr)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));

#ifdef _WIN32
    int connfd = ::accept(sockfd, sockaddr_cast(&addr), &addrlen);
    setNonBlock(connfd);
#else
    int connfd = ::accept4(sockfd,  sockaddr_cast(&addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0)
    {
#ifdef _WIN32
        errno = ::WSAGetLastError();
        SYSLOG(ERROR) << "sockets::accept";
#else
        int savedErrno = errno;
        SYSLOG(ERROR) << "sockets::accept";

        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = savedErrno; // expected errors
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG(FATAL) << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG(FATAL) << "unknown error of ::accept " << savedErrno;
                break;
        }
#endif
    }
    else if (peerAddr != nullptr)
    {
        peerAddr->setSockAddr(addr);
    }
    return connfd;
}

int connect(int sockfd, const InetAddress& addr)
{
    const struct sockaddr_in& sockAddr = addr.getSockAddr();
    return ::connect(sockfd, sockaddr_cast(&sockAddr),
                     static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

int read(int sockfd, void* buf, int len)
{
#ifdef _WIN32
    return ::recv(sockfd, (char*)buf, len, 0);
#else
    return ::read(sockfd, buf, len);
#endif
}

int write(int sockfd, const void* buf, int len)
{
#ifdef _WIN32
    return ::send(sockfd, (const char*)buf, len, 0);
#else
    return ::write(sockfd, buf, len);
#endif
}

void close(int sockfd)
{
#ifdef _WIN32
    if (::closesocket(sockfd) < 0)
#else
    if (::close(sockfd) < 0)
#endif
    {
        SYSLOG(ERROR) << "sockets::close";
    }
}

void shutdownWrite(int sockfd)
{
#ifdef _WIN32
    if (::shutdown(sockfd, SD_SEND) < 0)
#else
    if (::shutdown(sockfd, SHUT_WR) < 0)
#endif
    {
        SYSLOG(ERROR) << "sockets::shutdownWrite";
    }
}

void shutdownRead(int sockfd)
{
#ifdef _WIN32
    if (::shutdown(sockfd, SD_RECEIVE) < 0)
#else
    if (::shutdown(sockfd, SHUT_RD) < 0)
#endif
    {
        SYSLOG(ERROR) << "sockets::shutdownRead";
    }
}

void shutdownReadWrite(int sockfd)
{
#ifdef _WIN32
    if (::shutdown(sockfd, SD_BOTH) < 0)
#else
    if (::shutdown(sockfd, SHUT_RDWR) < 0)
#endif
    {
        SYSLOG(ERROR) << "sockets::shutdownReadWrite";
    }
}

void setTcpNoDelay(int sockfd, bool on)
{
    int optval = on ? 1 : 0;
#ifdef _WIN32
    ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)& optval, sizeof(optval));
#else
    ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
#endif
}

void setReuseAddr(int sockfd, bool on)
{
    int optval = on ? 1 : 0;
#ifdef _WIN32
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
#else
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
#endif
}

void setReusePort(int sockfd, bool on)
{
#ifdef _WIN32
    if (on)
    {
        LOG(ERROR) << "SO_REUSEPORT is not supported on Win32.";
    }
#else
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        SYSLOG(ERROR) << "SO_REUSEPORT failed.";
    }
#endif
}

int getSocketError(int sockfd)
{
    int optval;
#ifdef _WIN32
    int optlen = sizeof(optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) < 0)
    {
        return ::WSAGetLastError();
    }
#else
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
#endif
    return optval;
}

InetAddress getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen) < 0)
    {
        SYSLOG(ERROR) << "sockets::getLocalAddr";
    }
    return localaddr;
}

InetAddress getPeerAddr(int sockfd)
{
    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr), &addrlen) < 0)
    {
        SYSLOG(ERROR) << "sockets::getPeerAddr";
    }
    return peeraddr;
}

bool isSelfConnect(int sockfd)
{
    InetAddress localaddr = getLocalAddr(sockfd);
    InetAddress peeraddr = getPeerAddr(sockfd);
    return localaddr.portNetEndian() == peeraddr.portNetEndian() &&
           localaddr.ipNetEndian() == peeraddr.ipNetEndian();
}

} // namespace sockets

} // namespace net

