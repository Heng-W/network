
#include <stdio.h>
#include <unistd.h>
#include "util/logger.h"
#include "net/event_loop.h"
#include "net/tcp_server.h"

using namespace net;

using FilePtr = std::shared_ptr<FILE>;

const int kBufSize = 64 * 1024;
const char* g_file = nullptr;

void onConnection(const TcpConnectionPtr& conn)
{
    LOG(INFO) << "FileServer - " << conn->peerAddr().toIpPort() << " -> "
              << conn->localAddr().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected())
    {
        LOG(INFO) << "FileServer - Sending file " << g_file
                  << " to " << conn->peerAddr().toIpPort();
        conn->setHighWaterMarkCallback([](const TcpConnectionPtr & conn, size_t len)
        {
            LOG(INFO) << "HighWaterMark " << len;
        }, kBufSize + 1);

        FILE* fp = ::fopen(g_file, "rb");
        if (fp)
        {
            FilePtr ctx(fp, ::fclose);
            conn->setContext(ctx);
            char buf[kBufSize];
            size_t nread = ::fread(buf, 1, sizeof(buf), fp);
            conn->send(buf, static_cast<int>(nread));
        }
        else
        {
            conn->shutdown();
            LOG(INFO) << "FileServer - no such file";
        }
    }
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
    const FilePtr& fp = util::any_cast<const FilePtr&>(conn->getContext());
    char buf[kBufSize];
    size_t nread = ::fread(buf, 1, sizeof(buf), fp.get());
    if (nread > 0)
    {
        conn->send(buf, static_cast<int>(nread));
    }
    else
    {
        conn->shutdown();
        LOG(INFO) << "FileServer - done";
    }
}

int main(int argc, char* argv[])
{
    LOG(INFO) << "pid = " << getpid();
    if (argc > 1)
    {
        g_file = argv[1];

        EventLoop loop;
        InetAddress listenAddr(18825);
        TcpServer server(&loop, listenAddr);
        server.setName("FileServer");
        server.setConnectionCallback(onConnection);
        server.setWriteCompleteCallback(onWriteComplete);
        server.start();
        loop.loop();
    }
    else
    {
        fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
    }
}

