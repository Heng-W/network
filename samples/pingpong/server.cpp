
#include <stdio.h>
#include <unistd.h>
#include <utility>

#include "util/logger.h"
#include "net/event_loop.h"
#include "net/inet_address.h"
#include "net/tcp_server.h"

using namespace util;
using namespace net;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn->setTcpNoDelay(true);
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
    conn->send(*buf);
    buf->retrieveAll();
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    }
    else
    {
        LOG(INFO) << "pid = " << getpid() << ", tid = " << std::this_thread::get_id();
        Logger::setMinLogLevel(Logger::WARN);

        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listenAddr(ip, port);
        int threadCount = atoi(argv[3]);

        EventLoop loop;

        TcpServer server(&loop, listenAddr);

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1)
        {
            server.setThreadNum(threadCount);
        }

        server.start();

        loop.loop();
    }
}

