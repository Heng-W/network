
#include "media_session.h"

#include "net/socket.h"
#include "net/event_loop.h"
#include "net/tcp_connection.h"

#include "rtsp_request.h"
#include "rtsp_server_info.h"
#include "device_source.h"
#include "h264_source.h"

namespace net
{

constexpr int RTP_VESION = 2;
constexpr int kRtpHeaderSize = 12;

MediaSession::MediaSession(const std::shared_ptr<TcpConnection>& conn,
                           const RtspRequest& request,
                           RtspServerInfo* server)
    : conn_(conn),
      loop_(conn->getLoop()),
      session_(request.session()),
      rtpInfo_(request.rtpInfo()),
      clientIp_(conn->peerAddr().toIp()),
      server_(server)
{
    rtpHeaderInit();
    setPayloadType(RtpPayloadType::H264);
}


void MediaSession::rtpHeaderInit()
{
    rtpHeader_.csrcLen = 0;
    rtpHeader_.extension = 0;
    rtpHeader_.padding = 0;
    rtpHeader_.version = RTP_VESION;
    rtpHeader_.payloadType = RtpPayloadType::H264;
    rtpHeader_.marker = 0;
    rtpHeader_.seq = 0;
    rtpHeader_.timestamp = 0;
    rtpHeader_.ssrc = 0x88923423;
}

void MediaSession::writeRtpHeaderTo(char* buf)
{
    buf[0] = rtpHeader_.csrcLen | rtpHeader_.extension << 4 |
             rtpHeader_.padding << 5 | rtpHeader_.version << 6;
    buf[1] = static_cast<uint8_t>(rtpHeader_.payloadType) | rtpHeader_.marker << 7;
    *(uint16_t*)(buf + 2) = hostToNetwork16(rtpHeader_.seq);
    *(uint32_t*)(buf + 4) = hostToNetwork32(rtpHeader_.timestamp);
    *(uint32_t*)(buf + 8) = hostToNetwork32(rtpHeader_.ssrc);
}

bool MediaSession::send(Buffer* payload, int frameSize)
{
    if (rtpInfo_.isOverUdp)
        return sendOverUdp(payload, frameSize);
    else
        return sendOverTcp(payload, frameSize);
}

bool MediaSession::sendOverUdp(Buffer* payload, int len)
{
    assert(rtpInfo_.isOverUdp);

    char header[kRtpHeaderSize];
    writeRtpHeaderTo(header);

    payload->prepend(header, sizeof(header));

    auto conn = conn_.lock();
    if (conn && conn->connected())
    {
        sockets::sendByUdp(server_->rtpSocket->fd(), payload->peek(), sizeof(header) + len,
                           InetAddress(clientIp_, rtpInfo_.clientRtpPort));
        payload->retrieve(sizeof(header));
        ++rtpHeader_.seq;
        return true;
    }
    else
    {
        payload->retrieve(sizeof(header));
        return false;
    }
}

bool MediaSession::sendOverTcp(Buffer* payload, int len)
{
    assert(!rtpInfo_.isOverUdp);

    char header[4 + kRtpHeaderSize];
    header[0] = '$'; // 标识符，用于与RTSP区分
    header[1] = rtpInfo_.rtpChannel; // channel，用于区分RTP和RTCP
    header[2] = ((len + kRtpHeaderSize) & 0xFF00) >> 8; // RTP包的大小
    header[3] = (len + kRtpHeaderSize) & 0xFF;

    writeRtpHeaderTo(header + 4);

    payload->prepend(header, sizeof(header));

    auto conn = conn_.lock();
    if (conn && conn->connected())
    {
        conn->send(payload->peek(), sizeof(header) + len);
        payload->retrieve(sizeof(header));
        ++rtpHeader_.seq;
        return true;
    }
    else
    {
        payload->retrieve(sizeof(header));
        return false;
    }
}

void MediaSession::start()
{
    startTime_ = Timestamp::now();

    DeviceSource* deviceSource = server_->findSource(session_);
    if (deviceSource != nullptr)
    {
        deviceSource->addSession(shared_from_this());
        return;
    }

    auto timerId = std::make_shared<int64_t>(-1);

    H264Source source;
    source.setSendFrameCallback([this](Buffer * buf, int len) { return this->send(buf, len); });

    auto file = std::make_shared<H264FileReader>(session_, source);
    auto guard = shared_from_this();
    *timerId = loop_->addTimer([timerId, guard, file]
    {
        if (!file->handleFrame())
        {
            EventLoop* loop = guard->getLoop();
            loop->queueInLoop([loop, timerId]
            {
                if (*timerId >= 0)
                    loop->removeTimer(*timerId);
            });
            return;
        }
        guard->rtpHeader_.timestamp += 90000 / 25;
    }, 0, 1000 / 25);
}

} // namespace net
