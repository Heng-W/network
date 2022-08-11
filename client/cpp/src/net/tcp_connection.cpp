
#include "tcp_connection.h"

#include <errno.h>
#include "../util/logger.h"
#include "socket.h"

namespace net
{

constexpr int kMaxReadSize = 65536;

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    LOG(TRACE) << conn->localAddr().toIpPort() << " -> "
               << conn->peerAddr().toIpPort() << " is "
               << (conn->connected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf, Timestamp)
{
    buf->retrieveAll();
}

TcpConnection::TcpConnection(int sockfd, const InetAddress& peerAddr)
    : state_(kConnecting),
      sockfd_(sockfd),
      sendThreadId_(std::this_thread::get_id()),
      localAddr_(sockets::getLocalAddr(sockfd)),
      peerAddr_(peerAddr)
{
    LOG(DEBUG) << "TcpConnection::constructor[" << "] at " << this << " fd=" << sockfd;
}

TcpConnection::~TcpConnection()
{
    LOG(DEBUG) << "TcpConnection::destructor[" << "] at " << this
               << " fd=" << sockfd_
               << " state=" << stateToString();
    assert(state_ == kDisconnected);
    sockets::close(sockfd_);
}

void TcpConnection::send(const void* data, int len)
{
    if (state_ == kConnected)
    {
        if (isInSendThread())
        {
            sendInThread(data, len);
        }
        else
        {
            Buffer buf(len, 0);
            buf.append(data, len);
            buffersToSend_.put(std::move(buf));
        }
    }
}

void TcpConnection::send(const std::string& message)
{
    send(message.data(), message.size());
}

void TcpConnection::send(const Buffer& buf)
{
    send(buf.peek(), buf.readableBytes());
}

void TcpConnection::send(Buffer&& buf)
{
    if (state_ == kConnected)
    {
        if (isInSendThread())
        {
            sendInThread(buf.peek(), buf.readableBytes());
        }
        else
        {
            buffersToSend_.put(std::move(buf));
        }
    }
}

bool TcpConnection::sendInThread(const void* data, int len)
{
    int n = sockets::write(sockfd_, data, len);
    if (n < 0)
    {
        SYSLOG(ERROR) << "TcpConnection::sendInThread";
        int err = sockets::getSocketError(sockfd_);
        LOG(ERROR) << "socket error ["
                   << "] - SO_ERROR = " << err << " " << strerror(err);
        return false;
    }
    assert(n == len);
    if (writeCompleteCallback_ && buffersToSend_.size() == 0)
    {
        writeCompleteCallback_(shared_from_this());
    }
    return true;
}

void TcpConnection::doSendEvent()
{
    while (true)
    {
        Buffer buf = buffersToSend_.get();
        if (state_ == kDisconnected)
        {
            LOG(WARN) << "disconnected, give up writing";
            return;
        }
        if (buf.readableBytes() > 0)
        {
            if (!sendInThread(buf.peek(), buf.readableBytes()))
            {
                break;
            }
        }
        else if (state_ == kDisconnecting)
        {
            break;
        }
    }
    sockets::shutdownWrite(sockfd_); // send FIN
}


void TcpConnection::doRecvEvent()
{
    while (true)
    {
        inputBuffer_.ensureWritableBytes(kMaxReadSize);
        int n = sockets::read(sockfd_, inputBuffer_.beginWrite(), kMaxReadSize);
        if (n > 0)
        {
            inputBuffer_.hasWritten(n);
            messageCallback_(shared_from_this(), &inputBuffer_, Timestamp::now());
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            SYSLOG(ERROR) << "TcpConnection::doRecvEvent";
            int err = sockets::getSocketError(sockfd_);
            LOG(ERROR) << "socket error ["
                       << "] - SO_ERROR = " << err << " " << strerror(err);
            break;
        }
    }
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        buffersToSend_.put(Buffer(0)); // wakeup
    }
}


void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        buffersToSend_.put(Buffer(0)); // wakeup
    }
}

void TcpConnection::connectEstablished()
{
    assert(state_ == kConnecting);
    setState(kConnected);
}

void TcpConnection::connectDestroyed()
{
    setState(kDisconnected);
}

void TcpConnection::setTcpNoDelay(bool on)
{
    sockets::setTcpNoDelay(sockfd_, on);
}

const char* TcpConnection::stateToString() const
{
    State state = state_;
    switch (state)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

} // namespace net
