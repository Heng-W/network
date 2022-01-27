#ifndef NET_RPC_SERVICE_H
#define NET_RPC_SERVICE_H

#include "net_ext/message/message.h"

namespace net
{

struct RequestCallbackList
{
    std::function<MessagePtr()> createRequest;
    std::function<MessagePtr(const MessagePtr&)> handle;
};

namespace detail
{
void registerRpcService(const util::StringView& tag,
                        const std::shared_ptr<RequestCallbackList>& callbacks);
} // namespace detail

template <class Request, class Response>
inline void registerRpcService(const std::function<void(const Request&, Response*)>& cb)
{
    static_assert(std::is_base_of<Message, Request>::value,
                  "Request must be derived from net::Message.");
    static_assert(std::is_base_of<Message, Response>::value,
                  "Response must be derived from net::Message.");
    auto callbacks = std::make_shared<RequestCallbackList>();
    callbacks->createRequest = [] { return std::make_shared<Request>(); };
    callbacks->handle = [cb](const MessagePtr & msg) -> MessagePtr
    {
        auto request = util::down_pointer_cast<Request>(msg);
        assert(request);
        auto resp = std::make_shared<Response>();
        cb(*request, resp.get());
        return resp;
    };
    util::StringView tag = Request::kMessageTag;
    detail::registerRpcService(tag, std::move(callbacks));
}

void registerRpcRequestHandler();

} // namespace net

#endif // NET_RPC_SERVICE_H
