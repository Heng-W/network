
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
    return std::string(sdp);
}


bool RtspContext::handleRequest(const TcpConnectionPtr& conn, Buffer* buf, util::Timestamp)
{
    const char* start = buf->peek();

    // request line
    const char* crlf = buf->findCRLF();
    if (!crlf) return false;

    // method
    const char* space = std::find(start, crlf, ' ');
    if (space == crlf || !request_.setMethod(start, space)) return false;

    // url
    start = space + 1;
    space = std::find(start, crlf, ' ');
    if (space == crlf) return false;
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
    if (!(crlf - start == 8 && std::equal(start, crlf, "RTSP/1.0"))) return false;

    // next line
    buf->retrieveUntil(crlf + 2);
    start = crlf + 2;
    crlf = buf->findCRLF();
    if (!crlf) return false;

    // 解析序列号
    int cseq;
    if (sscanf(start, "CSeq: %d\r\n", &cseq) != 1) return false;
    request_.setCSeq(cseq);

    buf->retrieveUntil(crlf + 2);
    start = crlf + 2;
    while (true)
    {
        crlf = buf->findCRLF();
        if (!crlf) return false;
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf)
        {
            request_.addHeader(buf->peek(), colon, crlf);
            buf->retrieveUntil(crlf + 2);
        }
        else
        {
            buf->retrieveUntil(crlf + 2);
            break;
        }
    }

    LOG(TRACE) << "C--------->S\n" << start;

    RtspResponse resp;
    resp.setStatusCode(RtspResponse::k200Ok);
    resp.setStatusMessage("OK");
    resp.setCSeq(cseq);

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
