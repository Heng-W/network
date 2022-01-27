
#include "rpc_service.h"

#include <unordered_map>
#include "net/tcp_connection.h"
#include "util/logger.h"
#include "rpc.msg.h"

namespace net
{

static std::unordered_map<util::StringView, RequestCallbackList> s_callbackMap;

namespace detail
{

void registerRpcService(const util::StringView& tag,
                        RequestCallbackList&& callbacks)
{
    s_callbackMap[tag] = std::move(callbacks);
}

} // namespace detail

static void handleRequest(const net::TcpConnectionPtr& conn,
                          const std::shared_ptr<RpcRequest>& rpcRequest,
                          util::Timestamp receiveTime)
{
    RpcResponse rpcResp;
    rpcResp.id = rpcRequest->id;

    util::StringView tag(rpcRequest->method.data(), rpcRequest->method.size());
    auto it = s_callbackMap.find(tag);
    if (it == s_callbackMap.end())
    {
        rpcResp.status = -1;
        rpcResp.describe = "invalid method";
        send(conn, rpcResp);
        return;
    }
    auto handler = &it->second;
    assert(handler);
    auto request = handler->createRequest();
    assert(request);
    if (request->decodeFromBytes(rpcRequest->content.data(), rpcRequest->content.size()))
    {
        auto resp = handler->handle(std::move(request));

        rpcResp.status = 0;
        rpcResp.content.resize(resp->calcByteSize());
        size_t nwrote = resp->encodeToBytes(&*rpcResp.content.begin()); // 写入编码字节流
        (void)nwrote;
        assert(nwrote == rpcResp.content.size());
        send(conn, rpcResp);
    }
    else
    {
        rpcResp.status = -1;
        rpcResp.describe = "parse content";
        send(conn, rpcResp);
    }
}

void registerRpcRequestHandler()
{
    net::registerMessageHandler<RpcRequest>(handleRequest);
}

} // namespace net

