
#include <stdio.h>
#include <utility>
#include <atomic>

#include "util/logger.h"
#include "net/endian.h"
#include "net/inet_address.h"
#include "net/tcp_client.h"

using namespace net;


using FilePtr = std::shared_ptr<FILE>;

class FileClient
{
public:
    FileClient(const InetAddress& serverAddr)
        : client_(serverAddr),
          fileSize_(0)
    {
        client_.setConnectionCallback([this](const TcpConnectionPtr & conn)
        {
            LOG(INFO) << conn->localAddr().toIpPort() << " -> "
                      << conn->peerAddr().toIpPort() << " is "
                      << (conn->connected() ? "UP" : "DOWN");
            if (conn->connected())
            {
                FILE* fp = ::fopen("test.h264", "wb");
                if (fp)
                {
                    filePtr_ = FilePtr(fp, ::fclose);
                    LOG(INFO) << "start to recv file";
                }
                else
                {
                    conn->shutdown();
                    LOG(ERROR) << "open file";
                }
            }
            else
            {
                if (filePtr_) LOG(INFO) << "finish, recv size: " << fileSize_;
            }
        });
        client_.setMessageCallback([this](const TcpConnectionPtr & conn, Buffer * buf, Timestamp receiveTime)
        {
            (void)conn;
            LOG(DEBUG) << " recv size: " << buf->readableBytes()
                       << " at " << receiveTime.toFormattedString();
            ::fwrite(buf->peek(), 1, buf->readableBytes(), filePtr_.get());
            fileSize_ += buf->readableBytes();
            buf->retrieveAll();
        });
    }

    void start()
    {
        client_.start();
    }

private:

    TcpClient client_;
    FilePtr filePtr_;
    int fileSize_;
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s host_ip\n", argv[0]);
        return -1;
    }

    InetAddress serverAddr(argv[1], 18825);

    FileClient fileClient(serverAddr);
    fileClient.start();
    return 0;
}

