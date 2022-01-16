#ifndef NET_HTTP_REQUEST_H
#define NET_HTTP_REQUEST_H

#include <assert.h>
#include <map>
#include "util/timestamp.h"

namespace net
{

class HttpRequest
{
public:
    enum Method
    {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };

    enum Version
    {
        kUnknown, kHttp10, kHttp11
    };

    HttpRequest(): method_(kInvalid), version_(kUnknown) {}

    void swap(HttpRequest& rhs)
    {
        using std::swap;
        swap(method_, rhs.method_);
        swap(version_, rhs.version_);
        path_.swap(rhs.path_);
        query_.swap(rhs.query_);
        swap(receiveTime_, rhs.receiveTime_);
        headers_.swap(rhs.headers_);
    }

    void setVersion(Version v) { version_ = v; }
    Version version() const { return version_; }

    bool setMethod(const char* start, const char* end)
    {
        assert(method_ == kInvalid);
        std::string m(start, end);

        if (m == "GET") method_ = kGet;
        else if (m == "POST") method_ = kPost;
        else if (m == "HEAD") method_ = kHead;
        else if (m == "PUT") method_ = kPut;
        else if (m == "DELETE") method_ = kDelete;
        else method_ = kInvalid;

        return method_ != kInvalid;
    }

    Method method() const { return method_; }

    const char* methodString() const
    {
        const char* result = "UNKNOWN";
        switch (method_)
        {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
        }
        return result;
    }

    void setPath(const char* start, const char* end) { path_.assign(start, end); }
    const std::string& path() const { return path_; }

    void setQuery(const char* start, const char* end) { query_.assign(start, end); }
    const std::string& query() const { return query_; }

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


private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    util::Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;
};

} // namespace net

#endif // NET_HTTP_REQUEST_H

