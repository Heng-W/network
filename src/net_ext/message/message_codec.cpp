
#include "message_codec.h"

#include "net/buffer.h"
#include "net/tcp_connection.h"
#include "util/logger.h"
#include "message.h"

namespace net
{

namespace codec
{

namespace
{

const int kHeaderLen = sizeof(uint32_t);
const int kMinMessageLen = 5;
const int kMaxMessageLen = 64 * 1024 * 1024;

ErrorCallback errorCallback_ = [](const TcpConnectionPtr& conn, Buffer*, util::Timestamp, ErrorCode errorCode)
{
    LOG(ERROR) << "MessageCodec::defaultErrorCallback - " << errorCodeToString(errorCode);
    if (conn && conn->connected())
    {
        conn->shutdown();
    }
};

uint16_t asUInt16(const char* buf)
{
    uint32_t be16 = 0;
    ::memcpy(&be16, buf, sizeof(be16));
    return networkToHost16(be16);
}

uint16_t calcCheckSum(const void* buf, int size)
{
    unsigned int cksum = 0;
    const uint16_t* data = reinterpret_cast<const uint16_t*>(buf);
    while (size > 1)
    {
        cksum += *data++;
        size -= sizeof(uint16_t);
    }
    if (size)
    {
        cksum += *reinterpret_cast<const uint8_t*>(data);
    }
    // 将32位数转换成16位
    while (cksum >> 16)
    {
        cksum = (cksum >> 16) + (cksum & 0xffff);
    }
    return static_cast<uint16_t>(~cksum);
}

inline net::Buffer createBuffer(const Message& msg)
{
    int byteSize = msg.calcByteSize();
    util::StringView tag = msg.getTag();

    net::Buffer buf(kHeaderLen + 2 + tag.size() + byteSize + 2, kHeaderLen);

    buf.appendUInt16(tag.size()); // 消息tag长度
    buf.append(tag.data(), tag.size()); // 消息tag

    int nwrote = msg.encodeToBytes(buf.beginWrite()); // 写入编码字节流
    (void)nwrote;
    assert(nwrote == byteSize);
    buf.hasWritten(byteSize);

    uint16_t checkSum = calcCheckSum(buf.peek(), buf.readableBytes());
    buf.appendUInt16(checkSum);
    buf.prependUInt32(buf.readableBytes());

    return buf;
}

ErrorCode handle(const TcpConnectionPtr& conn, const char* buf, int len, Timestamp receiveTime)
{
    // check sum
    const char* end = buf + len;
    uint16_t expectedCheckSum = asUInt16(end - sizeof(uint16_t));
    uint16_t checkSum = calcCheckSum(buf, len - sizeof(uint16_t));
    if (checkSum != expectedCheckSum)
    {
        return ErrorCode::CHECK_SUM;
    }
    end -= sizeof(uint16_t);

    uint16_t tagLen = asUInt16(buf);
    buf += sizeof(uint16_t);
    auto handler = MessageHandlerDispatcher::instance()
                   .findHandlerByTag(util::StringView(buf, tagLen));
    buf += tagLen;

    if (!handler)
    {
        return ErrorCode::UNKNOWN_MESSAGE_TYPE;
    }
    MessagePtr msg = handler->createMessage();
    assert(msg);
    // parse from buffer
    if (msg->decodeFromBytes(buf, end - buf))
    {
        handler->handle(conn, msg, receiveTime);
        return ErrorCode::SUCCESS;
    }
    else
    {
        return ErrorCode::PARSE_MESSAGE;
    }
}

} // namespace

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    while (buf->readableBytes() >= kMinMessageLen + kHeaderLen)
    {
        const uint32_t len = buf->peekUInt32();
        if (len > kMaxMessageLen || len < kMinMessageLen)
        {
            errorCallback_(conn, buf, receiveTime, ErrorCode::INVALID_LENGTH);
            break;
        }
        else if (buf->readableBytes() >= len + kHeaderLen)
        {
            ErrorCode errorCode = handle(conn, buf->peek() + kHeaderLen, len, receiveTime);
            if (errorCode == ErrorCode::SUCCESS)
            {
                buf->retrieve(kHeaderLen + len);
            }
            else
            {
                errorCallback_(conn, buf, receiveTime, errorCode);
                break;
            }
        }
        else
        {
            break;
        }
    }
}

void setErrorCallback(const ErrorCallback& cb)
{
    errorCallback_ = cb;
}

std::string errorCodeToString(ErrorCode errorCode)
{
    switch (errorCode)
    {
        case ErrorCode::SUCCESS:
            return "NoError";
        case ErrorCode::INVALID_LENGTH:
            return "InvalidLength";
        case ErrorCode::CHECK_SUM:
            return "CheckSumError";
        case ErrorCode::UNKNOWN_MESSAGE_TYPE:
            return "UnknownMessageType";
        case ErrorCode::PARSE_MESSAGE:
            return "ParseError";
        default:
            return "UnknownError";
    }
}

} // namespace codec

void send(const TcpConnectionPtr& conn, const Message& msg)
{
    conn->send(codec::createBuffer(msg));
}

Buffer createBuffer(const Message& msg)
{
    return codec::createBuffer(msg);
}

} // namespace net
