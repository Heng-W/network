
#include "rpc.msg.h"
#include "net/tcp_connection.h"
#include "util/logger.h"

namespace net
{

static void handleRequest(const net::TcpConnectionPtr& conn,
                          const std::shared_ptr<RpcRequest>& rpcRequest,
                          util::Timestamp receiveTime)
{
    util::StringView tag(rpcRequest->method.data(), rpcRequest->method.size());
    LOG(INFO)<<tag;
    auto handler = MessageHandlerDispatcher::instance().findHandlerByTag(tag);
    RpcResponse rpcResp;
    rpcResp.id = rpcRequest->id;
 LOG(INFO)<<rpcRequest->id;
    if (!handler)
    {
        rpcResp.status = -1;
        rpcResp.describe = "invalid method";
        send(conn, rpcResp);
        return;
    }
    MessagePtr msg = handler->createMessage();
    assert(msg);
    if (msg->decodeFromBytes(rpcRequest->content.data(),rpcRequest->content.size()))
    {
        handler->handle(conn, msg, receiveTime);
        const MessagePtr& resp = util::any_cast<const MessagePtr&>(conn->getContext());
       
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

