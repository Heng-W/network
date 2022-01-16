
#include "time.h"
#include "util/logger.h"
#include "net/endian.h"
#include "net_ext/message/message_codec.h"

TimeServer::TimeServer(net::EventLoop* loop,
                       const net::InetAddress& listenAddr)
    : server_(loop, listenAddr)
{
    server_.setConnectionCallback([](const net::TcpConnectionPtr & conn)
    {
        LOG(INFO) << "TimeServer - " << conn->peerAddr().toIpPort() << " -> "
                  << conn->localAddr().toIpPort() << " is "
                  << (conn->connected() ? "UP" : "DOWN");
        if (conn->connected())
        {
            time_t now = ::time(NULL);
            int32_t be32 = net::hostToNetwork32(static_cast<int32_t>(now));
            conn->send(&be32, sizeof be32);
        }
    });
    server_.setMessageCallback(&net::codec::onMessage);
}

