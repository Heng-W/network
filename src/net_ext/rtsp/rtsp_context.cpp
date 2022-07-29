
#include "rtsp_context.h"

#include "util/logger.h"
#include "net/tcp_connection.h"
#include "rtsp_response.h"
#include "rtsp_server_info.h"
#include "media_session.h"

namespace net
{

static std::string getSdpString(const RtspRequest& request)
{
    char localIp[32];
    sscanf(request.url().c_str(), "rtsp://%[^:]:", localIp);

    char sdp[256];
    snprintf(sdp, sizeof(sdp), "v=0\r\n"
             "o=- 9%ld 1 IN IP4 %s\r\n"
             "t=0 0\r\n"
             "a=control:*\r\n"
             "m=video 0 RTP/AVP 96\r\n"
             "a=rtpmap:96 H264/90000\r\n"
             "a=control:track0\r\n",
             time(nullptr), localIp);
    return sdp;
}


bool RtspContext::processRequestLine(const char* start, const char* end)
{
    // method
    const char* space = std::find(start, end, ' ');
    if (space == end || !request_.setMethod(start, space)) return false;

    // url
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space == end) return false;
    request_.setUrl(start, space);

    // get session from url
    if (!std::equal(start, start + 7, "rtsp://")) return false;
    start += 7;
    const char* colon = std::find(start, space, ':');
    if (colon == space) return false;
    start = std::find(colon + 1, space, '/');
    if (start != space)
    {
        request_.setSession(start + 1, space);
    }

    // version
    start = space + 1;
    if (!(end - start == 8 && std::equal(start, end, "RTSP/1.0"))) return false;
    return true;
}



bool RtspContext::handleRequest(const TcpConnectionPtr& conn, Buffer* buf, util::Timestamp receiveTime)
{
    bool hasMore = true;
    while (hasMore)
    {
        if (state_ == kExpectRequestLine)
        {
            const char* crlf = buf->findCRLF();
            if (!crlf) return true;
            bool ok = processRequestLine(buf->peek(), crlf);
            if (!ok) return false;
            request_.setReceiveTime(receiveTime);
            buf->retrieveUntil(crlf + 2);
            state_ = kExpectCSeq;
        }
        else if (state_ == kExpectCSeq)
        {
            const char* crlf = buf->findCRLF();
            if (!crlf) return true;
            const char* colon = std::find(buf->peek(), crlf, ':');
            if (!std::equal(buf->peek(), colon, "CSeq")) return false;
            ++colon;
            while (colon < crlf && isspace(*colon)) ++colon;
            int cseq = 0;
            while (colon < crlf && isdigit(*colon))
            {
                cseq = cseq * 10 + (*colon - '0');
                ++colon;
            }
            request_.setCSeq(cseq);
            buf->retrieveUntil(crlf + 2);
            state_ = kExpectHeaders;
        }
        else if (state_ == kExpectHeaders)
        {
            const char* crlf = buf->findCRLF();
            if (!crlf) return true;
            const char* colon = std::find(buf->peek(), crlf, ':');
            if (colon != crlf)
            {
                request_.addHeader(buf->peek(), colon, crlf);
            }
            else
            {
                // empty line, end of header
                state_ = kExpectRequestLine;;
                hasMore = false;
            }
            buf->retrieveUntil(crlf + 2);
        }
    }

    RtspResponse resp;
    resp.setStatusCode(RtspResponse::k200Ok);
    resp.setStatusMessage("OK");
    resp.setCSeq(request_.cseq());

    switch (request_.method())
    {
        case RtspRequest::kOptions:
            resp.addHeader("Public", RtspRequest::allMethodsToString());
            break;
        case RtspRequest::kDescribe:
            resp.addHeader("Content-Base", request_.url());
            resp.addHeader("Content-type", "application/sdp");
            resp.setBody(getSdpString(request_));
            break;
        case RtspRequest::kSetup:
        {
            const std::string& transportStr = request_.getHeader("Transport");
            const char* transport = transportStr.c_str();
            if (std::equal(transport, transport + 8, "RTP/AVP;"))
            {
                RtpInfo info;
                info.isOverUdp = true;
                if (sscanf(transport,
                           "RTP/AVP;unicast;client_port=%hu-%hu",
                           &info.clientRtpPort, &info.clientRtcpPort) != 2) return false;
                request_.setRtpInfo(info);

                char tmp[128];
                snprintf(tmp, sizeof(tmp),
                         "RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu",
                         info.clientRtpPort, info.clientRtcpPort,
                         server_->rtpPort, server_->rtcpPort);
                resp.addHeader("Transport", tmp);
            }
            else if (std::equal(transport, transport + 12, "RTP/AVP/TCP;"))
            {
                RtpInfo info;
                info.isOverUdp = false;
                if (sscanf(transport,
                           "RTP/AVP/TCP;unicast;interleaved=%hhu-%hhu",
                           &info.rtpChannel, &info.rtcpChannel) != 2) return false;
                request_.setRtpInfo(info);
                char tmp[128];
                snprintf(tmp, sizeof(tmp),
                         "RTP/AVP/TCP;unicast;interleaved=%hhu-%hhu",
                         info.rtpChannel,
                         static_cast<uint8_t>(info.rtpChannel + 1));
                resp.addHeader("Transport", tmp);
            }
            else
            {
                return false;
            }

            resp.addHeader("Session", "66334873");
            break;
        }
        case RtspRequest::kPlay:
        {
            resp.addHeader("Range", "npt=0.000-");
            resp.addHeader("Session", "66334873; timeout=60");
            break;
        }
        case RtspRequest::kTeardown:
            session_.reset();
            break;
        default:
            return false;
    }

    Buffer sendBuf;
    resp.appendToBuffer(&sendBuf);
    LOG(TRACE) << "S--------->C\n"
               << std::string(sendBuf.peek(), sendBuf.readableBytes());
    conn->send(std::move(sendBuf));

    if (request_.method() == RtspRequest::kPlay)
    {
        session_ = std::make_shared<MediaSession>(conn, request_, server_);
        session_->start();
        LOG(TRACE) << "start play";
    }
    return true;
}

} // namespace net
