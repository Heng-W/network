#ifndef NET_MEDIA_SESSION_H
#define NET_MEDIA_SESSION_H

#include <memory>
#include "rtp.h"
#include "util/timestamp.h"

namespace net
{

class EventLoop;
class TcpConnection;
class Buffer;
class RtspRequest;
struct RtspServerInfo;

class MediaSession: public std::enable_shared_from_this<MediaSession>
{
public:

    MediaSession(const std::shared_ptr<TcpConnection>& conn,
                 const RtspRequest& request, RtspServerInfo* server);

    void start();
    bool send(Buffer* payload, int frameSize);

    void updateTimestamp()
    {
        util::Timestamp t = util::Timestamp::now();
        uint32_t ms = (t - startTime_).toMsec();
        rtpHeader_.timestamp = ms * 90;
    }

    EventLoop* getLoop() { return loop_; }

private:

    void setPayloadType(RtpPayloadType payloadType)
    { rtpHeader_.payloadType = payloadType; }

    void rtpHeaderInit();
    void writeRtpHeaderTo(char* buf);

    bool sendOverUdp(Buffer* payload, int len);
    bool sendOverTcp(Buffer* payload, int len);

    std::weak_ptr<TcpConnection> conn_;
    EventLoop* loop_;
    std::string session_;
    RtpHeader rtpHeader_;

    RtpInfo rtpInfo_;
    std::string clientIp_;
    RtspServerInfo* server_;

    util::Timestamp startTime_;
};

} // namespace net

#endif // NET_MEDIA_SESSION_H
