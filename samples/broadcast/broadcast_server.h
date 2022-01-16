#ifndef BROADCAST_SERVER_H
#define BROADCAST_SERVER_H

#include <set>
#include "net/tcp_server.h"

namespace net
{
class Buffer;
}

class BroadcastServer
{
public:
    using EventCallback = std::function<void(const net::TcpConnectionPtr&)>;

    BroadcastServer(net::EventLoop* loop, const net::InetAddress& listenAddr);

    void start() { server_.start(); }

    void setMessageCallback(const net::MessageCallback& cb)
    { server_.setMessageCallback(cb); }


    void broadcast(const void* data, int len);
    void broadcast(std::string&& message);
    void broadcast(net::Buffer&& buf);

    void broadcast(const EventCallback& cb);

private:
    using TcpConnectionWeakptr = std::weak_ptr<net::TcpConnection>;

    void broadcastInLoop(const EventCallback& cb);
    void broadcastInLoop(const void* data, int len);

    net::TcpServer server_;
    std::set<TcpConnectionWeakptr, std::owner_less<TcpConnectionWeakptr>> connections_;
};

#endif // BROADCAST_SERVER_H
