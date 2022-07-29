#ifndef NET_MESSAGE_SSL_CODEC_H
#define NET_MESSAGE_SSL_CODEC_H

#include <memory>
#include <functional>
#include "util/timestamp.h"

namespace net
{

struct Message;
class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;


namespace ssl
{
namespace codec
{

enum class ErrorCode
{
    SUCCESS = 0,
    INVALID_LENGTH,
    CHECK_SUM,
    UNKNOWN_MESSAGE_TYPE,
    PARSE_MESSAGE,
};

using ErrorCallback = std::function<void(const TcpConnectionPtr&, Buffer*, util::Timestamp, ErrorCode)>;

void setErrorCallback(const ErrorCallback& cb);

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, util::Timestamp receiveTime);

std::string errorCodeToString(ErrorCode errorCode);

} // namespace codec

void send(const TcpConnectionPtr& conn, const Message& message);

} // namespace ssl
} // namespace net

#endif // NET_MESSAGE_SSL_CODEC_H
