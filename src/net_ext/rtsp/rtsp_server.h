#ifndef NET_RTSP_SERVER_H
#define NET_RTSP_SERVER_H

#include "net/tcp_server.h"

namespace net
{

class RtspRequest;
class RtspResponse;
class DeviceSource;
class RtspServerInfo;

// simple rtsp server
class RtspServer
{
public:
    using RtspCallback = std::function<void(const RtspRequest&, RtspResponse*)>;

    RtspServer(EventLoop* loop,
               const InetAddress& listenAddr = InetAddress(8554),
               const InetAddress& rtpSocketAddr = InetAddress(8556),
               const InetAddress& rtcpSocketAddr = InetAddress(8557),
               TcpServer::Option option = TcpServer::kNoReusePort);
    ~RtspServer();

    // 在调用start()前设置（非线程安全）
    void setRtspCallback(const RtspCallback& cb) { rtspCallback_ = cb; }

    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    EventLoop* getLoop() const { return server_.getLoop(); }

    void start();

    void addDeviceSource(const std::string& session, DeviceSource* source);

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);

    TcpServer server_;
    RtspCallback rtspCallback_;

    std::unique_ptr<RtspServerInfo> info_;
};

} // namespace net

#endif // NET_RTSP_SERVER_H
