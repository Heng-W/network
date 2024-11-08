#ifndef NET_RTSP_RESPONSE_H
#define NET_RTSP_RESPONSE_H

#include <map>
#include <string>

namespace net
{

class Buffer;

class RtspResponse
{
public:
    enum RtspStatusCode
    {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit RtspResponse(): statusCode_(kUnknown) {}

    void setStatusCode(RtspStatusCode code) { statusCode_ = code; }
    void setStatusMessage(const std::string& message) { statusMessage_ = message; }
    void setCSeq(int cseq) { cseq_ = cseq; }

    void addHeader(const std::string& key, const std::string& value)
    { headers_[key] = value; }

    void setBody(const std::string& body) { body_ = body; }

    void appendToBuffer(Buffer* output) const;

private:
    std::map<std::string, std::string> headers_;
    RtspStatusCode statusCode_;
    std::string statusMessage_;
    int cseq_;
    std::string body_;
};

} // namespace net

#endif // NET_RTSP_RESPONSE_H
