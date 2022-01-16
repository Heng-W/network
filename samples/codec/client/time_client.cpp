
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utility>

#include "util/logger.h"
#include "net/endian.h"
#include "net/event_loop.h"
#include "net/inet_address.h"
#include "net/tcp_client.h"
#include "net/channel.h"

#include "simple.msg.h"


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
    }

    void connect()
    {
        client_.connect();
    }

    TcpConnectionPtr connection() const
    {
        return client_.connection();
    }

private:

    EventLoop* loop_;
    TcpClient client_;
};


inline void setnonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
    {
        perror("get fcntl flag");
        return;
    }
    int ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK | O_CLOEXEC);
    if (ret == -1)
    {
        perror("set fcntl non-blocking");
        return;
    }
}


int main(int argc, char* argv[])
{
    LOG(INFO) << "pid = " << getpid();
    if (argc > 1)
    {
        EventLoop loop;
        InetAddress serverAddr(argv[1], 2037);

        TimeClient timeClient(&loop, serverAddr);
        timeClient.connect();


        setnonblock(STDIN_FILENO);
        Channel inputChannel(&loop, STDIN_FILENO);
        inputChannel.setReadCallback([&loop, &timeClient](Timestamp)
        {
            char buf[256];
            if (read(STDIN_FILENO, buf, sizeof(buf)) > 0)
            {
                strtok(buf, "\n");
                if (strcmp(buf, "q") == 0)
                {
                    loop.quit();
                }
                else
                {
                    LOG(INFO) << buf;
                    SimpleMessage msg;
                    msg.value = buf[0];

                    auto connectionPtr = timeClient.connection();
                    if (connectionPtr) send(connectionPtr, msg);
                }
            }
        });
        inputChannel.enableReading();

        loop.loop();
    }
    else
    {
        printf("Usage: %s host_ip\n", argv[0]);
    }
}

