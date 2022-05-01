
#include "tcp_client.h"

#include "../util/logger.h"
#include "connector.h"
#include "socket.h"

namespace net
{

TcpClient::TcpClient(const InetAddress& serverAddr)
    : connector_(new Connector(serverAddr)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      quit_(false),
      recv_(false),
      connect_(true),
      retry_(false),
      name_("Tcp Client"),
      baseThreadId_(std::this_thread::get_id()),
      recvThread_{&TcpClient::recvThreadFunc, this}
{
    connector_->setNewConnectionCallback([this](int sockfd) { this->newConnection(sockfd); });
    LOG(INFO) << "TcpClient::TcpClient[" << name_
              << "] - connector " << connector_.get();
}

TcpClient::~TcpClient()
{
    LOG(INFO) << "TcpClient::~TcpClient[" << name_
              << "] - connector " << connector_.get();

    quit_ = true;
    recvCond_.notify_one();
    recvThread_.join();
}

void TcpClient::start()
{
    assert(isInBaseThread());
    LOG(INFO) << "TcpClient::connect[" << name_ << "] - connecting to "
              << connector_->serverAddress().toIpPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::stop()
{
    connect_ = false;

    TcpConnectionPtr conn;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        conn = connection_;
    }
    if (conn)
    {
        conn->shutdown();
    }
    else
    {
        connector_->stop();
    }
}

void TcpClient::setRetryDelay(int delayMs, bool fixed)
{
    connector_->setRetryDelay(delayMs, fixed);
}

void TcpClient::newConnection(int sockfd)
{
    InetAddress peerAddr = sockets::getPeerAddr(sockfd);
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(sockfd, peerAddr);

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    if (!connect_) return;
    conn->connectEstablished();

    recv_ = true;
    recvCond_.notify_one();

    conn->doSendEvent();

    {
        std::unique_lock<std::mutex> lock(mutex_);
        recvCond_.wait(lock, [this] { return !recv_ || quit_; });
        connection_.reset();
    }

    conn->connectDestroyed();
    conn.reset();

    if (retry_ && connect_)
    {
        LOG(INFO) << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                  << connector_->serverAddress().toIpPort();
        connector_->start();
    }
}


void TcpClient::recvThreadFunc()
{
    while (!quit_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            recvCond_.wait(lock, [this] { return recv_ || quit_; });
        }
        if (quit_) return;

        connection_->doRecvEvent();

        recv_ = false;
        recvCond_.notify_one();
    }
}

} // namespace net

