#ifndef NET_H264_SOURCE_H
#define NET_H264_SOURCE_H

#include <functional>
#include <string>
#include "net/buffer.h"

namespace net
{

class H264Source
{
public:
    using SendFrameCallback = std::function<bool(Buffer*, int)>;


    bool handleFrame(Buffer* frame, int frameSize);

    void setSendFrameCallback(const SendFrameCallback& cb)
    { sendFrameCallback_ = cb; }

private:
    SendFrameCallback sendFrameCallback_;
};


class H264FileReader
{
public:
    H264FileReader(const std::string& fileName, const H264Source& source);
    ~H264FileReader();

    bool handleFrame()
    {
        if (!readFrame()) return false;
        return source_.handleFrame(&frame_, frameSize_);
    }

private:
    bool readFrame();

    int fd_;
    Buffer frame_;
    int frameSize_;
    H264Source source_;
};

} // namespace net

#endif // NET_H264_SOURCE_H
