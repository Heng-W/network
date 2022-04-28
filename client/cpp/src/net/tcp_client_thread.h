#ifndef NET_TCP_CLIENT_THREAD_H
#define NET_TCP_CLIENT_THREAD_H

#include "util/count_down_latch.h"
#include "tcp_client.h"

namespace net
{

class TcpClientThread
{
public:
    TcpClientThread(const InetAddress& serverAddr)
        : start_(false),
          quit_(false)
    {
        util::CountDownLatch latch(1);
        thread_.reset(new std::thread([&latch, &serverAddr, this]()
        {
            TcpClient client(serverAddr);
            client_ = &client;
            latch.countDown();

            {
                std::unique_lock<std::mutex> lock(mutex_);
                cond_.wait(lock, [this] { return start_ || quit_; });
            }
            if (quit_) return;

            client.start();

            {
                std::unique_lock<std::mutex> lock(mutex_);
                cond_.wait(lock, [this] { return quit_ == true; });
            }
        }));
        latch.wait();
    }

    ~TcpClientThread()
    {
        stop();
        quit_ = true;
        cond_.notify_one();
        thread_->join();
    }

    DISALLOW_COPY_AND_ASSIGN(TcpClientThread);

    void start() { start_ = true; cond_.notify_one(); }
    void stop() { client_->stop(); }

    void setRetryDelay(int delayMs, bool fixed)
    { client_->setRetryDelay(delayMs, fixed); }

    TcpConnectionPtr connection() const { return client_->connection(); }

    bool retry() const { return client_->retry(); }
    void enableRetry() { client_->enableRetry(); }

    void setName(const std::string& name) { client_->setName(name); }
    const std::string& name() const { return client_->name(); }

    // 设置回调函数（非线程安全）
    void setConnectionCallback(const ConnectionCallback& cb)
    { client_->setConnectionCallback(cb); }

    void setMessageCallback(const MessageCallback& cb)
    { client_->setMessageCallback(cb); }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { client_->setWriteCompleteCallback(cb); }

private:
    TcpClient* client_;
    std::unique_ptr<std::thread> thread_;

    std::atomic<bool> start_, quit_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
};

} // namespace net

#endif // NET_TCP_CLIENT_THREAD_H

