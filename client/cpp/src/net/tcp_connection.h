#ifndef NET_TCP_CONNECTION_H
#define NET_TCP_CONNECTION_H

#include <atomic>
#include <thread>
#include "../util/common.h"
#include "../util/blocking_queue.hpp"
#include "../util/any.hpp"
#include "callbacks.h"
#include "buffer.h"
#include "inet_address.h"

namespace net
{

// TCP连接
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(int sockfd, const InetAddress& peerAddr);
    ~TcpConnection();

    DISALLOW_COPY_AND_ASSIGN(TcpConnection);

    void send(const void* data, int len);
    void send(const std::string& message);

    void send(const Buffer& buf);
    void send(Buffer&& buf);

    void shutdown();

    void connectEstablished();
    void connectDestroyed();

    void setTcpNoDelay(bool on);

    void doSendEvent();
    void doRecvEvent();

    // set callbacks
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    const InetAddress& localAddr() const { return localAddr_; }
    const InetAddress& peerAddr() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    Buffer* inputBuffer() { return &inputBuffer_; }

    void setContext(const util::Any& context) { context_ = context; }
    void setContext(util::Any&& context) { context_ = std::move(context); }
    const util::Any& getContext() const { return context_; }
    util::Any* getMutableContext() { return &context_; }

private:
    enum State {kDisconnected, kConnecting, kConnected, kDisconnecting};

    bool sendInThread(const void* data, int len);

    bool isInSendThread() const { return sendThreadId_ == std::this_thread::get_id(); }

    void setState(State state) { state_ = state; }

    const char* stateToString() const;


    std::atomic<State> state_;
    int sockfd_;
    std::thread::id sendThreadId_;

    InetAddress localAddr_;
    InetAddress peerAddr_;

    util::BlockingQueue<Buffer> buffersToSend_;

    // callbacks
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    Buffer inputBuffer_;

    util::Any context_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace net

#endif // NET_TCP_CONNECTION_H

