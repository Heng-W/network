
#include "rpc_client_service.h"

#include <atomic>
#include <unordered_map>
#include <mutex>
#include "util/logger.h"
#include "net_ext/message/message_codec.h"
#include "rpc.msg.h"

namespace net
{

namespace
{
std::atomic<uint64_t> s_id;
std::unordered_map<uint64_t, std::shared_ptr<ResponseCallbackList>> s_callbackMap;
std::mutex s_mutex;

} // namespace

namespace detail
{

void rpcSolve(const TcpConnectionPtr& conn,
              const Message& request,
              const std::shared_ptr<ResponseCallbackList>& callbacks)
{
    uint64_t id = ++s_id;
    RpcRequest rpcRequest;
    rpcRequest.id = id;
    rpcRequest.method = request.getTag().toString();
    rpcRequest.content.resize(request.calcByteSize());
    size_t nwrote = request.encodeToBytes(&*rpcRequest.content.begin()); // 写入编码字节流
    (void)nwrote;
    assert(nwrote == rpcRequest.content.size());
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_callbackMap[id] = callbacks;
    }
    send(conn, rpcRequest);
}

} // namespace detail

static void handleResponse(const net::TcpConnectionPtr& conn,
                           const std::shared_ptr<RpcResponse>& rpcResponse,
                           util::Timestamp receiveTime)
{
    std::shared_ptr<ResponseCallbackList> callbacks;

    {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_callbackMap.find(rpcResponse->id);
        if (it != s_callbackMap.end())
        {
            callbacks = std::move(it->second);
            s_callbackMap.erase(it);
        }
    }
    if (!callbacks)
    {
        LOG(ERROR) << "unknown id: " << rpcResponse->id;
        return;
    }
    auto msg = callbacks->createResponse();
    assert(msg);
    if (msg->decodeFromBytes(rpcResponse->content.data(), rpcResponse->content.size()))
    {
        callbacks->handle(msg);
    }
    else
    {
        LOG(ERROR) << "parse response error";
    }
}

void registerRpcResponseHandler()
{
    net::registerMessageHandler<RpcResponse>(handleResponse);
}

} // namespace net
