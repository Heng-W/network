
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
    RpcResponse resp;
    resp.id = rpcRequest->id;
    if (!handler)
    {
        resp.status = -1;
        resp.describe = "invalid method";
        send(conn, resp);
        return;
    }
    MessagePtr msg = handler->createMessage();
    assert(msg);
    if (msg->decodeFromBytes(rpcRequest->content.data(),rpcRequest->content.size()))
    {
        handler->handle(conn, msg, receiveTime);
        resp.status = 0;
        resp.content.resize(msg->calcByteSize());
        size_t nwrote = msg->encodeToBytes(&*resp.content.begin()); // 写入编码字节流
        (void)nwrote;
        assert(nwrote == resp.content.size());
        send(conn, resp);
    }
    else
    {
        resp.status = -1;
        resp.describe = "parse content";
        send(conn, resp);
    }
}

void registerRpcRequestHandler()
{
    net::registerMessageHandler<RpcRequest>(handleRequest);
}

} // namespace net

