#ifndef NET_HTTP_CONTEXT_H
#define NET_HTTP_CONTEXT_H

#include "http_request.h"

namespace net
{

class Buffer;

class HttpContext
{
public:
    enum HttpRequestParseState
    {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpContext() : state_(kExpectRequestLine) {}

    bool parseRequest(Buffer* buf, util::Timestamp receiveTime);

    bool gotAll() const { return state_ == kGotAll; }

    void reset()
    {
        state_ = kExpectRequestLine;
        HttpRequest().swap(request_);
    }

    const HttpRequest& request() const { return request_; }
    HttpRequest& request() { return request_; }

private:
    bool processRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;
    HttpRequest request_;
};

} // namespace net

#endif // NET_HTTP_CONTEXT_H

