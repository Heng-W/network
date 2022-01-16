
#include "h264_source.h"

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include "util/logger.h"
#include "rtp.h"

namespace net
{

inline bool startCode3(const char* buf)
{
    return buf[0] == 0 && buf[1] == 0 && buf[2] == 1;
}

inline bool startCode4(const char* buf)
{
    return buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1;
}

static const char* findNextStartCode(const char* buf, int len)
{
    if (len < 3) return nullptr;

    for (int i = 0; i < len - 3; ++i)
    {
        if (startCode3(buf) || startCode4(buf)) return buf;
        ++buf;
    }
    if (startCode3(buf)) return buf;
    return nullptr;
}

H264FileReader::H264FileReader(const std::string& fileName,
                               const H264Source& source)
    :  frame_(65536, 64),
       frameSize_(0),
       source_(source)
{
    fd_ = open(fileName.c_str(), O_RDONLY);
    assert(fd_ >= 0);
}

H264FileReader::~H264FileReader()
{
    close(fd_);
    LOG(TRACE) << "close h264";
}

bool H264FileReader::readFrame()
{
    if (frame_.readableBytes() == 0)
    {
        frame_.ensureWritableBytes(8192);
        int n = ::read(fd_, frame_.beginWrite(), frame_.writableBytes());
        if (n <= 0) return false;
        frame_.hasWritten(n);
        const char* frame = frame_.peek();
        if (!startCode3(frame) && !startCode4(frame)) return false;
    }

    int startCode = startCode3(frame_.peek()) ? 3 : 4;

    const char* nextStartCode = findNextStartCode(frame_.peek() + startCode, frame_.readableBytes() - startCode);
    while (!nextStartCode)
    {
        frame_.ensureWritableBytes(8192);
        int n = ::read(fd_, frame_.beginWrite(), frame_.writableBytes());
        if (n == 0) // reach end of file
        {
            nextStartCode = frame_.beginWrite();
            break;
        }
        else if (n < 0 || frame_.readableBytes() > 1024 * 500)
        {
            return false;
        }
        nextStartCode = findNextStartCode(frame_.beginWrite(), n);
        frame_.hasWritten(n);
    }
    frameSize_ = nextStartCode - frame_.peek();
    return true;
}

bool H264Source::handleFrame(Buffer* frame, int frameSize)
{
    const char* frameData = frame->peek();
    const char* end = frameData + frameSize;

    int startCode = startCode3(frameData) ? 3 : 4;

    frameSize -= startCode;
    frameData += startCode;
    frame->retrieve(startCode);

    uint8_t naluType = frameData[0]; // nalu第一个字节

    if (frameSize <= kRtpMaxPacketSize) // nalu长度小于最大包长：单一NALU单元模式
    {
        /**
         *   0 1 2 3 4 5 6 7 8 9
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  |F|NRI|  Type   | a single NAL unit ... |
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         */
        if (sendFrameCallback_)
        {
            if (!sendFrameCallback_(frame, frameSize))
            {
                frame->retrieve(frameSize);
                return false;
            }
        }

        frame->retrieve(frameSize);
    }
    else // nalu长度大于最大包长：分片模式
    {
        /**
         *  0                   1                   2
         *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         * | FU indicator  |   FU header   |   FU payload   ...  |
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *
         *     FU Indicator
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |F|NRI|  Type   |
         *   +---------------+
         *
         *      FU Header
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |S|E|R|  Type   |
         *   +---------------+
         */

        int pktNum = frameSize / kRtpMaxPacketSize;
        int remainPktSize = frameSize % kRtpMaxPacketSize;

        frame->retrieve(1); // skip nalu header

        for (int i = 0; i < pktNum; ++i)
        {
            char header[2];
            header[0] = (naluType & 0x60) | 28;
            header[1] = naluType & 0x1F;

            if (i == 0) //第一包数据
                header[1] |= 0x80; // start
            else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
                header[1] |= 0x40; // end

            frame->prepend(header, sizeof(header));

            if (sendFrameCallback_)
            {
                if (!sendFrameCallback_(frame, kRtpMaxPacketSize + sizeof(header)))
                {
                    frame->retrieve(end - frame->peek());
                    return false;
                }
            }
            frame->retrieve(kRtpMaxPacketSize + sizeof(header));
        }

        // 发送剩余的数据
        if (remainPktSize > 0)
        {
            char header[2];
            header[0] = (naluType & 0x60) | 28;
            header[1] = naluType & 0x1F;
            header[1] |= 0x40; // end

            frame->prepend(header, sizeof(header));

            if (sendFrameCallback_)
            {
                if (!sendFrameCallback_(frame, remainPktSize + sizeof(header)))
                {
                    frame->retrieve(end - frame->peek());
                    return false;
                }
            }
            frame->retrieve(remainPktSize + sizeof(header));
        }
    }
    return true;
}

} // namespace net
