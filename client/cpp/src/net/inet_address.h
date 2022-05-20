#ifndef NET_INET_ADDRESS_H
#define NET_INET_ADDRESS_H

#ifdef _WIN32
#include <WinSock2.h>
#ifdef ERROR
#undef ERROR
#endif
typedef int sa_family_t;
typedef int socklen_t;
typedef unsigned int in_addr_t;
#else
#include <netinet/in.h>
#endif // _WIN32
#include <string>

namespace net
{

class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

    InetAddress(const std::string& ip, uint16_t port);
    InetAddress(const struct sockaddr_in& addr): addr_(addr) {}

    sa_family_t family() const { return addr_.sin_family; }
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const struct sockaddr_in& getSockAddr() const { return addr_; }
    void setSockAddr(const struct sockaddr_in& addr) { addr_ = addr; }

    uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }
    uint16_t portNetEndian() const { return addr_.sin_port; }

    static bool resolve(const std::string& hostname, InetAddress* result);

private:
    struct sockaddr_in addr_;
};

} // namespace net

#endif // NET_INET_ADDRESS_H
