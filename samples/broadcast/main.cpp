
#include "broadcast_server.h"
#include "net/event_loop.h"


int main()
{
    net::EventLoop loop;
    BroadcastServer server(&loop, net::InetAddress(2037));
    server.start();
    loop.loop();
    return 0;
}