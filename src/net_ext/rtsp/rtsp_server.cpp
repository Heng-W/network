
#include "rtsp_server.h"

#include "util/logger.h"
#include "net/socket.h"
#include "net/channel.h"
#include "rtsp_context.h"
#include "rtsp_response.h"
#include "rtsp_server_info.h"

namespace net
{

namespace
{

void defaultRtspCallback(const RtspRequest&, RtspResponse* resp)
{
    resp->setStatusCode(RtspResponse::kUnknown);
    resp->setStatusMessage("invalid");
}

} // namespace

RtspServer::RtspServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const InetAddress& rtpSocketAddr,
                       const InetAddress& rtcpSocketAddr,
                       TcpServer::Option option)
    : server_(loop, listenAddr, option),
      rtspCallback_(defaultRtspCallback),
      info_(new RtspServerInfo())
{
    server_.setConnectionCallback([this](const TcpConnectionPtr & conn) { this->onConnection(conn); });
    server_.setMessageCallback(
        [this](const TcpConnectionPtr & conn, Buffer * buf, Timestamp receiveTime)
    {
        this->onMessage(conn, buf, receiveTime);
    });

    info_->rtpSocket.reset(new Socket(sockets::createUdpNonBlock()));
    info_->rtcpSocket.reset(new Socket(sockets::createUdpNonBlock()));

    info_->rtpSocket->bind(rtpSocketAddr);
    info_->rtcpSocket->bind(rtcpSocketAddr);

    info_->rtpPort = rtpSocketAddr.toPort();
    info_->rtcpPort = rtcpSocketAddr.toPort();

    info_->rtpChannel.reset(new Channel(loop, info_->rtpSocket->fd()));
    info_->rtpChannel->setReadCallback([this](Timestamp)
    {
        char buf[4];
        int fd = info_->rtpSocket->fd();
        InetAddress peerAddr;
        sockets::recvByUdp(fd, buf, sizeof(buf), &peerAddr);
        uint16_t portNetEndian = peerAddr.portNetEndian();
        memcpy(buf, &portNetEndian, sizeof(portNetEndian));
        sockets::sendByUdp(fd, buf, sizeof(uint16_t), peerAddr);
    });
    info_->rtpChannel->enableReading();
}

RtspServer::~RtspServer()
{
    info_->rtpChannel->disableAll();
    info_->rtpChannel->remove();
}

void RtspServer::start()
{
    LOG(INFO) << "RtspServer[" << server_.name()
              << "] starts listening on " << server_.ipPort();
    server_.start();
}

void RtspServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn->setContext(RtspContext(info_.get()));
    }
}

void RtspServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    RtspContext* context = util::any_cast<RtspContext>(conn->getMutableContext());

    context->handleRequest(conn, buf, receiveTime);
}

void RtspServer::addDeviceSource(const std::string& session, DeviceSource* source)
{
    info_->addDeviceSource(session, source);
}

} // namespace net
