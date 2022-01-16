
#include <stdio.h>
#include <utility>
#include <atomic>

#include "util/logger.h"
#include "net/endian.h"
#include "net/inet_address.h"
#include "net/tcp_client.h"

using namespace net;


class TimeClient
{
public:
    TimeClient(const InetAddress& serverAddr)
        : client_(serverAddr)
    {
        client_.setConnectionCallback([](const TcpConnectionPtr & conn)
        {
            LOG(INFO) << conn->localAddr().toIpPort() << " -> "
                      << conn->peerAddr().toIpPort() << " is "
                      << (conn->connected() ? "UP" : "DOWN");
            if (!conn->connected())
            {
                LOG(INFO) << "disconnected";
            }
        });
        client_.setMessageCallback([](const TcpConnectionPtr & conn, Buffer * buf, Timestamp receiveTime)
        {
            (void)conn;
            if (buf->readableBytes() >= sizeof(int32_t))
            {
                const void* data = buf->peek();
                int32_t be32 = *static_cast<const int32_t*>(data);
                buf->retrieve(sizeof(int32_t));
                time_t time = net::networkToHost32(be32);
                Timestamp ts(static_cast<uint64_t>(time) * Timestamp::kMicroSecondsPerSecond);
                LOG(INFO) << "Server time = " << time << ", " << ts.toFormattedString();
            }
            else
            {
                LOG(INFO) << " no enough data " << buf->readableBytes()
                          << " at " << receiveTime.toFormattedString();
            }
        });
        // client_.enableRetry();
    }

    void start()
    {
        client_.start();
    }

private:

    TcpClient client_;
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s host_ip\n", argv[0]);
        return -1;
    }

    InetAddress serverAddr(argv[1], 2037);

    TimeClient client(serverAddr);

    while (true)
    {
        client.start();
        if (getchar() == 'q') break;
    }
    return 0;
}

