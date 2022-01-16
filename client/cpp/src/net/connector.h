#ifndef NET_CONNECTOR_H
#define NET_CONNECTOR_H

#include <functional>
#include <memory>
#include <atomic>
#include "../util/common.h"
#include "inet_address.h"

namespace net
{

class Connector
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)> ;

    Connector(const InetAddress& serverAddr);
    ~Connector();

    DISALLOW_COPY_AND_ASSIGN(Connector);

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void setRetryDelay(int delayMs, bool fixed)
    { retryDelayMs_ = delayMs; fixedRetryDelay_ = fixed; }

    void start();
    void stop(); // 线程安全

    const InetAddress& serverAddress() const { return serverAddr_; }

private:
    enum States { kDisconnected, kConnecting, kConnected };

    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) { state_ = s; }

    void doConnectEvent();
    int tryConnect();
    int connecting(int sockfd);

    void createWakeupfd();
    void wakeup();
    void handleRead();

    InetAddress serverAddr_;
    std::atomic<bool> connect_;
    States state_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
    bool fixedRetryDelay_;

#ifdef _WIN32
    int wakeupSendFd_;
    int wakeupRecvFd_;
#else
    int wakeupFd_;
#endif
};

} // namespace net

#endif // NET_CONNECTOR_H

