#ifndef NET_RPC_SERVICE_H
#define NET_RPC_SERVICE_H

#include "net_ext/message/message.h"

namespace net
{

struct RequestCallbackList
{
    std::function<std::unique_ptr<Message>()> createRequest;
    std::function<std::unique_ptr<Message>(std::unique_ptr<Message>)> handle;
};

namespace detail
{
void registerRpcService(const util::StringView& tag,
                        RequestCallbackList&& callbacks);
} // namespace detail

template <class Request, class Response>
inline void registerRpcService(const std::function<void(const Request&, Response*)>& cb)
{
    static_assert(std::is_base_of<Message, Request>::value,
                  "Request must be derived from net::Message.");
    static_assert(std::is_base_of<Message, Response>::value,
                  "Response must be derived from net::Message.");
    RequestCallbackList callbacks;
    callbacks.createRequest = [] { return std::unique_ptr<Request>(new Request()); };
    callbacks.handle = [cb](std::unique_ptr<Message> msg) -> std::unique_ptr<Message>
    {
        auto request = util::down_pointer_cast<Request>(std::move(msg));
        assert(request);
        std::unique_ptr<Response> resp(new Response());
        cb(*request, resp.get());
        return resp;
    };
    util::StringView tag = Request::kMessageTag;
    detail::registerRpcService(tag, std::move(callbacks));
}

void registerRpcRequestHandler();

} // namespace net

#endif // NET_RPC_SERVICE_H
