
#include "echo.h"
#include "util/logger.h"


EchoServer::EchoServer(net::EventLoop* loop,
                       const net::InetAddress& listenAddr)
    : server_(loop, listenAddr)
{
    server_.setConnectionCallback([](const net::TcpConnectionPtr & conn)
    {
        LOG(INFO) << "EchoServer - " << conn->peerAddr().toIpPort() << " -> "
                  << conn->localAddr().toIpPort() << " is "
                  << (conn->connected() ? "UP" : "DOWN");
    });
    server_.setMessageCallback([](const net::TcpConnectionPtr & conn,
                                  net::Buffer * buf,
                                  net::Timestamp time)
    {
        std::string msg(buf->retrieveAllAsString());
        LOG(INFO) << conn->id() << " echo " << msg.size() << " bytes, "
                  << "data received at " << time.toString();
        conn->send(std::move(msg));
    });
}

