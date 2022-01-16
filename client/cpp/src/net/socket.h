#ifndef NET_SOCKET_H
#define NET_SOCKET_H

namespace net
{

class InetAddress;

namespace sockets
{

int createTcp();
int createTcpNonBlock();
int createUdp();
int createUdpNonBlock();
void setNonBlock(int sockfd, bool on = true);

void bind(int sockfd, const InetAddress& addr);
void listen(int sockfd);
int accept(int sockfd, InetAddress* peerAddr = nullptr);

int connect(int sockfd, const InetAddress& addr);

int read(int sockfd, void* buf, int len);
int write(int sockfd, const void* buf, int len);
void close(int sockfd);

void shutdownWrite(int sockfd);
void shutdownRead(int sockfd);
void shutdownReadWrite(int sockfd);

void setTcpNoDelay(int sockfd, bool on);
void setReuseAddr(int sockfd, bool on);
void setReusePort(int sockfd, bool on);

int getSocketError(int sockfd);

InetAddress getLocalAddr(int sockfd);
InetAddress getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);

} // namespace sockets

class Socket
{
public:
    explicit Socket(int sockfd): sockfd_(sockfd) {}
    ~Socket() { sockets::close(sockfd_); }

    Socket(const Socket&) = delete;

    void bind(const InetAddress& localAddr) { sockets::bind(sockfd_, localAddr); }
    void listen() { sockets::listen(sockfd_); }
    int accept(InetAddress* peerAddr = nullptr) { return sockets::accept(sockfd_, peerAddr); }

    void shutdownWrite() { sockets::shutdownWrite(sockfd_); }

    void setTcpNoDelay(bool on) { sockets::setTcpNoDelay(sockfd_, on); }
    void setReuseAddr(bool on) { sockets::setReuseAddr(sockfd_, on); }
    void setReusePort(bool on) { sockets::setReusePort(sockfd_, on); }

    int fd() const { return sockfd_; }

private:
    const int sockfd_;
};

} // namespace net

#endif // NET_SOCKET_H
