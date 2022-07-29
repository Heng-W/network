#ifndef NET_RTSP_CONTEXT_H
#define NET_RTSP_CONTEXT_H

#include "net/callbacks.h"
#include "rtsp_request.h"

namespace net
{

class Buffer;
class MediaSession;
struct RtspServerInfo;

class RtspContext
{
public:
    enum RtspRequestParseState
    {
        kExpectRequestLine,
        kExpectCSeq,
        kExpectHeaders,
        kGotAll,
    };

    RtspContext(RtspServerInfo* server): state_(kExpectRequestLine), server_(server) {}

    bool handleRequest(const TcpConnectionPtr& conn,
                       Buffer* buf,
                       util::Timestamp receiveTime);

    void reset()
    {
        state_ = kExpectRequestLine;
        RtspRequest().swap(request_);
    }

    const RtspRequest& request() const { return request_; }
    RtspRequest& request() { return request_; }

    std::shared_ptr<MediaSession> session() const { return session_; }

private:
    bool processRequestLine(const char* start, const char* end);

    RtspRequestParseState state_;
    RtspRequest request_;
    std::shared_ptr<MediaSession> session_;
    RtspServerInfo* server_;
};

} // namespace net

#endif // NET_RTSP_CONTEXT_H

