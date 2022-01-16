#ifndef NET_TCP_CLIENT_H
#define NET_TCP_CLIENT_H

#include "tcp_connection.h"

namespace net
{

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient
{
public:
    TcpClient(const InetAddress& serverAddr);
    ~TcpClient();

    DISALLOW_COPY_AND_ASSIGN(TcpClient);

    void start(); // 在创建TcpClient的线程中调用，该调用阻塞
    void stop(); // 线程安全

    void setRetryDelay(int delayMs, bool fixed);

    TcpConnectionPtr connection() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

    void setName(const std::string& name) { name_ = name; }
    const std::string& name() const { return name_; }

    // 设置回调函数（非线程安全）
    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    void recvThreadFunc();

    bool isInBaseThread() const
    { return std::this_thread::get_id() == baseThreadId_; }


    ConnectorPtr connector_;
    TcpConnectionPtr connection_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    std::atomic<bool> quit_;
    std::atomic<bool> recv_;
    std::atomic<bool> connect_;
    bool retry_;
    std::string name_;

    std::thread::id baseThreadId_;
    std::thread recvThread_;
    mutable std::mutex mutex_;
    std::condition_variable recvCond_;
};

} // namespace net

#endif // NET_TCP_CLIENT_H

