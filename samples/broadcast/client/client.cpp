
#include <stdio.h>
#include <unistd.h>
#include <utility>

#include "util/logger.h"
#include "net/endian.h"
#include "net/event_loop.h"
#include "net/inet_address.h"
#include "net/tcp_client.h"



using namespace net;


int main(int argc, char* argv[])
{
    std::string hostIp = "127.0.0.1";
    if (argc > 1) hostIp = argv[1];

    EventLoop loop;
    InetAddress serverAddr(hostIp, 2037);

    TcpClient client(&loop, serverAddr);
    client.setConnectionCallback([&loop](const TcpConnectionPtr & conn)
    {
        LOG(INFO) << conn->localAddr().toIpPort() << " -> "
                  << conn->peerAddr().toIpPort() << " is "
                  << (conn->connected() ? "UP" : "DOWN");

        if (!conn->connected())
        {
            loop.quit();
        }
    });
    client.setMessageCallback([&loop](const TcpConnectionPtr & conn, Buffer * buf, Timestamp receiveTime)
    {
        std::string msg(buf->retrieveAllAsString());
        LOG(INFO) << "conn" << conn->id() << ": " << msg << ", "
                  << "data received at " << receiveTime.toFormattedString();
    });
    client.connect();
    loop.loop();
}
