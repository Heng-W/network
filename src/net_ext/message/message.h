#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H

#include <memory>
#include <functional>
#include <unordered_map>
#include "util/timestamp.h"
#include "util/type.h"
#include "util/string_view.h"

namespace net
{

struct Message
{
    virtual ~Message() = default;

    virtual util::StringView getTag() const = 0;

    virtual int encodeToBytes(char* buf) const = 0;
    virtual bool decodeFromBytes(const char* buf, int len) = 0;

    int calcByteSize() const { return cachedSize_ = byteSize(); }
    int cachedSize() const { return cachedSize_; }

protected:
    virtual int byteSize() const = 0;

    mutable int cachedSize_ = 0;
};


class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using MessagePtr = std::shared_ptr<Message>;

struct MessageHandler
{
    virtual ~MessageHandler() = default;
    virtual void handle(const TcpConnectionPtr&,
                        const MessagePtr&,
                        util::Timestamp) const = 0;
    virtual MessagePtr createMessage() const = 0;
};

template <class T>
struct MessageHandlerT: MessageHandler
{
    static_assert(std::is_base_of<Message, T>::value,
                  "T must be derived from net::Message.");

    using HandleMessageCallback = std::function<void(const TcpConnectionPtr&,
                                  const std::shared_ptr<T>&,
                                  util::Timestamp)>;

    MessageHandlerT(const HandleMessageCallback& cb)
        : handleMessageCallback_(cb) {}

    void handle(const TcpConnectionPtr& conn,
                const MessagePtr& message,
                util::Timestamp receiveTime) const override
    {
        std::shared_ptr<T> concrete = util::down_pointer_cast<T>(message);
        assert(concrete);
        handleMessageCallback_(conn, concrete, receiveTime);
    }

    MessagePtr createMessage() const override { return std::make_shared<T>(); }

private:
    HandleMessageCallback handleMessageCallback_;
};

class MessageHandlerDispatcher
{
public:
    std::shared_ptr<MessageHandler> findHandlerByTag(const util::StringView& tag)
    {
        auto it = handlers_.find(tag);
        return it != handlers_.end() ? it->second : nullptr;
    }

    template <class T>
    void registerHandler(const typename MessageHandlerT<T>::HandleMessageCallback& cb)
    {
        auto handler = std::make_shared<MessageHandlerT<T>>(cb);
        util::StringView tag = T::kMessageTag;
        bool succeed = handlers_.insert({tag, handler}).second;
        if (!succeed) throw std::runtime_error("registerHandler");
    }

    static MessageHandlerDispatcher& instance()
    {
        static MessageHandlerDispatcher _instance;
        return _instance;
    }

private:
    MessageHandlerDispatcher() = default;

    using HandlerMap = std::unordered_map<util::StringView, std::shared_ptr<MessageHandler>>;

    HandlerMap handlers_;
};

template <class T>
inline void registerMessageHandler(const typename MessageHandlerT<T>::HandleMessageCallback& cb)
{
    net::MessageHandlerDispatcher::instance().registerHandler<T>(cb);
}


void send(const TcpConnectionPtr& conn, const Message& msg);

class Buffer;
Buffer createBuffer(const Message& msg);


} // namespace net


// 定义消息tag
#define MESSAGE_TAG(tag) \
    util::StringView getTag() const override { return kMessageTag; } \
    static constexpr util::StringView kMessageTag = (#tag);

// 注册消息处理回调
#define REGISTER_MESSAGE_HANDLER(MessageType, handleMessageCb) \
    static bool s_registerMessageHandlerOf ## MessageType = [] \
            {  \
               net::registerMessageHandler<MessageType>(handleMessageCb); \
               return true; \
            }();

#endif // NET_MESSAGE_H
