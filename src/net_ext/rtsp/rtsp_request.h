#ifndef NET_RTSP_REQUEST_H
#define NET_RTSP_REQUEST_H

#include <assert.h>
#include <map>
#include "util/timestamp.h"
#include "rtp.h"

namespace net
{

class RtspRequest
{
public:
    enum Method
    {
        kInvalid, kOptions, kDescribe, kSetup, kPlay, kTeardown
    };

    enum Version
    {
        kUnknown, kRtsp10
    };

    RtspRequest(): method_(kInvalid), version_(kUnknown) {}

    void swap(RtspRequest& rhs)
    {
        using std::swap;
        swap(method_, rhs.method_);
        swap(version_, rhs.version_);
        session_.swap(rhs.session_);
        swap(receiveTime_, rhs.receiveTime_);
        headers_.swap(rhs.headers_);
    }

    void setVersion(Version v) { version_ = v; }
    Version version() const { return version_; }

    bool setMethod(const char* start, const char* end)
    {
        assert(method_ == kInvalid);
        std::string m(start, end);

        if (m == "OPTIONS") method_ = kOptions;
        else if (m == "DESCRIBE") method_ = kDescribe;
        else if (m == "SETUP") method_ = kSetup;
        else if (m == "PLAY") method_ = kPlay;
        else if (m == "TEARDOWN") method_ = kTeardown;
        else method_ = kInvalid;

        return method_ != kInvalid;
    }

    Method method() const { return method_; }

    const char* methodString() const
    {
        const char* result = "UNKNOWN";
        switch (method_)
        {
            case kOptions:
                result = "OPTIONS";
                break;
            case kDescribe:
                result = "DESCRIBE";
                break;
            case kSetup:
                result = "SETUP";
                break;
            case kPlay:
                result = "PLAY";
                break;
            case kTeardown:
                result = "TEARDOWN";
                break;
            default:
                break;
        };

        return result;
    }

    void setUrl(const char* start, const char* end) { url_.assign(start, end); }
    const std::string& url() const { return url_; }


    void setSession(const char* start, const char* end) { session_.assign(start, end); }
    const std::string& session() const { return session_; }

    void setCSeq(int cseq) { cseq_ = cseq; }
    int cseq() const { return cseq_; }

    void setRtpInfo(const RtpInfo& info) { rtpInfo_ = info; }
    RtpInfo rtpInfo() const { return rtpInfo_; }

    void setReceiveTime(util::Timestamp t) { receiveTime_ = t; }
    util::Timestamp receiveTime() const { return receiveTime_; }

    void addHeader(const char* start, const char* colon, const char* end)
    {
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon))
        {
            ++colon;
        }
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size() - 1]))
        {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }

    std::string getHeader(const std::string& field) const
    {
        auto it = headers_.find(field);
        return it != headers_.end() ? it->second : "";
    }

    const std::map<std::string, std::string>& headers() const { return headers_; }

    static const char* allMethodsToString()
    { return "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN"; }

private:
    Method method_;
    Version version_;
    std::string url_;
    std::string session_;
    int cseq_;
    RtpInfo rtpInfo_;
    util::Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;
};

} // namespace net

#endif // NET_RTSP_REQUEST_H

