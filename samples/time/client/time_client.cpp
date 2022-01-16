
#include "util/logger.h"
#include "net/endian.h"
#include "net/event_loop.h"
#include "net/inet_address.h"
#include "net/tcp_client.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace net;

class TimeClient
{
public:
    TimeClient(EventLoop* loop, const InetAddress& serverAddr)
        : loop_(loop),
          client_(loop, serverAddr)
    {
        client_.setConnectionCallback([this](const TcpConnectionPtr & conn)
        {
            LOG(INFO) << conn->localAddr().toIpPort() << " -> "
                      << conn->peerAddr().toIpPort() << " is "
                      << (conn->connected() ? "UP" : "DOWN");

            if (!conn->connected())
            {
                loop_->quit();
            }
        });
        client_.setMessageCallback([](const TcpConnectionPtr & conn, Buffer * buf, Timestamp receiveTime)
        {
            if (buf->readableBytes() >= sizeof(int32_t))
            {
                const void* data = buf->peek();
                int32_t be32 = *static_cast<const int32_t*>(data);
                buf->retrieve(sizeof(int32_t));
                time_t time = net::networkToHost32(be32);
                Timestamp ts(util::implicit_cast<uint64_t>(time) * Timestamp::kMicroSecondsPerSecond);
                LOG(INFO) << "Server time = " << time << ", " << ts.toFormattedString();
            }
            else
            {
                LOG(INFO) << conn->id() << " no enough data " << buf->readableBytes()
                          << " at " << receiveTime.toFormattedString();
            }
        });
        // client_.enableRetry();
    }

    void connect()
    {
        client_.connect();
    }

private:

    EventLoop* loop_;
    TcpClient client_;
};

int main(int argc, char* argv[])
{
    LOG(INFO) << "pid = " << getpid();
    if (argc > 1)
    {
        EventLoop loop;
        InetAddress serverAddr(argv[1], 2037);

        TimeClient timeClient(&loop, serverAddr);
        timeClient.connect();
        loop.loop();
    }
    else
    {
        printf("Usage: %s host_ip\n", argv[0]);
    }
}

