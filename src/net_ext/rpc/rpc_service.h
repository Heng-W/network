#ifndef NET_RPC_SERVICE_H
#define NET_RPC_SERVICE_H

#include "net_ext/message/message.h"

namespace net
{
    
template <class Request, class Response>
inline void registerRpcService(const std::function<void(const Request&, Response*)>& call)
{
    registerMessageHandler<Request>([call](const TcpConnectionPtr & conn,
                                         const std::shared_ptr<Request>& request,
                                         util::Timestamp)
    {
        Response resp;
        call(*request, &resp);
        send(conn, resp);
    });
}

void registerRpcRequestHandler();

} // namespace net

#endif // NET_RPC_SERVICE_H
