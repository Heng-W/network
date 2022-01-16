#ifndef NET_ENDIAN_H
#define NET_ENDIAN_H

#include <stdint.h>
#ifdef _WIN32
#include <WinSock2.h>
#ifdef ERROR
#undef ERROR
#endif
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif // _MSC_VER
#else
#include <endian.h>
#endif // _WIN32

namespace net
{

inline uint64_t hostToNetwork64(uint64_t host64)
{
#ifdef _WIN32
    if (1 == htonl(1)) return host64;
    return (((uint64_t)htonl(host64)) << 32) + htonl(host64 >> 32);
#else
    return htobe64(host64);
#endif
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
#ifdef _WIN32
    return htonl(host32);
#else
    return htobe32(host32);
#endif
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
#ifdef _WIN32
    return htons(host16);
#else
    return htobe16(host16);
#endif
}

inline uint64_t networkToHost64(uint64_t net64)
{
#ifdef _WIN32
    if (1 == htonl(1)) return net64;
    return (((uint64_t)ntohl(net64)) << 32) + ntohl(net64 >> 32);
#else
    return be64toh(net64);
#endif
}

inline uint32_t networkToHost32(uint32_t net32)
{
#ifdef _WIN32
    return ntohl(net32);
#else
    return be32toh(net32);
#endif
}

inline uint16_t networkToHost16(uint16_t net16)
{
#ifdef _WIN32
    return ntohs(net16);
#else
    return be16toh(net16);
#endif
}

} // namespace net

#endif // NET_ENDIAN_H
