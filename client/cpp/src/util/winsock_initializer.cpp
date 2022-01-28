#ifdef _WIN32

#include "winsock_initializer.h"

#include <WinSock2.h>
#ifdef ERROR
#undef ERROR
#endif
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif // _MSC_VER


namespace detail
{

WinsockInitializer::WinsockInitializer()
{
    WORD versionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    ::WSAStartup(versionRequested, &wsaData);
}

WinsockInitializer::~WinsockInitializer()
{
    ::WSACleanup();
}

} // namespace detail

#endif // _WIN32