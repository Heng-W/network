
#include "rtsp_response.h"

#include <stdio.h>
#include "net/buffer.h"

namespace net
{

void RtspResponse::appendToBuffer(Buffer* output) const
{
    char buf[32];
    snprintf(buf, sizeof(buf), "RTSP/1.0 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_);
    output->append("\r\n");

    snprintf(buf, sizeof(buf), "CSeq: %d\r\n", cseq_);
    output->append(buf);

    if (!body_.empty())
    {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output->append(buf);
    }

    for (const auto& header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(body_);
}

} // namespace net
