
#include "broadcast_server.h"
#include "util/logger.h"
#include "net/event_loop.h"
#include "net/buffer.h"

using namespace net;

BroadcastServer::BroadcastServer(EventLoop* loop,
                                 const InetAddress& listenAddr)
    : server_(loop, listenAddr)
{
    server_.setConnectionCallback([this](const TcpConnectionPtr & conn)
    {
        LOG(INFO) << "Server - " << conn->peerAddr().toIpPort() << " -> "
                  << conn->localAddr().toIpPort() << " is "
                  << (conn->connected() ? "UP" : "DOWN");
        if (conn->connected())
        {
            broadcast("new connection " + std::to_string(conn->id()));
            server_.getLoop()->runInLoop([this, conn] { connections_.insert(conn); });
        }
        else
        {
            server_.getLoop()->runInLoop([this, conn] { connections_.erase(conn); });
            broadcast("close connection " + std::to_string(conn->id()));
        }
    });
    server_.setMessageCallback([](const TcpConnectionPtr & conn,
                                  Buffer * buf,
                                  Timestamp time)
    {
        std::string msg(buf->retrieveAllAsString());
        LOG(INFO) << conn->id() << " data " << msg << ", "
                  << "data received at " << time.toString();
    });
}

void BroadcastServer::broadcast(const EventCallback& cb)
{
    if (server_.getLoop()->isInLoopThread())
    {
        broadcastInLoop(cb);
    }
    else
    {
        server_.getLoop()->queueInLoop([this, cb]
        {
            broadcastInLoop(cb);
        });
    }
}

void BroadcastServer::broadcast(const void* data, int len)
{
    if (server_.getLoop()->isInLoopThread())
    {
        broadcastInLoop(data, len);
    }
    else
    {
        auto buf = std::make_shared<Buffer>(len, 0);
        buf->append(data, len);
        server_.getLoop()->queueInLoop([this, buf]
        {
            broadcastInLoop(buf->peek(), buf->readableBytes());
        });
    }
}

void BroadcastServer::broadcast(std::string&& message)
{
    if (server_.getLoop()->isInLoopThread())
    {
        broadcastInLoop(message.data(), message.size());
    }
    else
    {
        auto msg = std::make_shared<std::string>(std::move(message));
        server_.getLoop()->queueInLoop([this, msg]
        {
            broadcastInLoop(msg->data(), msg->size());
        });
    }
}

void BroadcastServer::broadcast(Buffer&& buf)
{
    if (server_.getLoop()->isInLoopThread())
    {
        broadcastInLoop(buf.peek(), buf.readableBytes());
    }
    else
    {
        auto bufPtr = std::make_shared<Buffer>(std::move(buf));
        server_.getLoop()->queueInLoop([this, bufPtr]
        {
            broadcastInLoop(bufPtr->peek(), bufPtr->readableBytes());
        });
    }
}

void BroadcastServer::broadcastInLoop(const EventCallback& cb)
{
    for (const auto& x : connections_)
    {
        auto guard = x.lock();
        if (guard) cb(guard);
    }
}

void BroadcastServer::broadcastInLoop(const void* data, int len)
{
    assert(server_.getLoop()->isInLoopThread());
    for (const auto& x : connections_)
    {
        auto guard = x.lock();
        if (guard) guard->send(data, len);
    }
}