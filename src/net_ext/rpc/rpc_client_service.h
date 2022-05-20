#ifndef NET_RPC_CLIENT_SERVICE_H
#define NET_RPC_CLIENT_SERVICE_H

#include <future>
#include "net_ext/message/message.h"

namespace net
{

template <class Response>
using HandleResponseCallback = std::function<void(const std::shared_ptr<Response>&)>;

struct ResponseCallbackList
{
    std::function<MessagePtr()> createResponse;
    HandleResponseCallback<Message> handle;
};

namespace detail
{
void rpcSolve(const TcpConnectionPtr& conn,
              const Message& request,
              const std::shared_ptr<ResponseCallbackList>& callbacks);
} // namespace detail

template <class Response>
void rpcSolve(const TcpConnectionPtr& conn,
              const Message& request,
              const HandleResponseCallback<Response>& cb)
{
    static_assert(std::is_base_of<Message, Response>::value,
                  "Response must be derived from net::Message.");
    auto callbacks = std::make_shared<ResponseCallbackList>();
    callbacks->createResponse = [] { return std::make_shared<Response>(); };
    callbacks->handle = [cb](const MessagePtr & message)
    {
        std::shared_ptr<Response> concrete = util::down_pointer_cast<Response>(message);
        assert(concrete);
        cb(concrete);
    };
    detail::rpcSolve(conn, request, std::move(callbacks));
}

template <class Response>
std::future<std::shared_ptr<Response>> 
rpcSolve(const TcpConnectionPtr& conn, const Message& request)
{
    using ResponsePtr = std::shared_ptr<Response>;
    auto p = std::make_shared<std::promise<ResponsePtr>>();
    auto f = p->get_future();
    rpcSolve<Response>(conn, request, [p](const ResponsePtr& msg)
    {
        p->set_value(msg);
    });
    return f;
}

void registerRpcResponseHandler();

} // namespace net

#endif // NET_RPC_CLIENT_SERVICE_H