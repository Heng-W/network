
#include "query.msg.h"

#include "util/logger.h"
#include "net/event_loop.h"
#include "net/tcp_server.h"
#include "net_ext/message/message_codec.h"
#include "net_ext/rpc/rpc_service.h"

int main()
{
    using namespace net;
    using namespace test;
    EventLoop loop;
    TcpServer server(&loop, InetAddress(18825));
    server.setMessageCallback(&codec::onMessage);
    registerRpcRequestHandler();

    registerRpcService<Query, Answer>([](const Query & request, Answer * response)
    {
        LOG(INFO) << "onAnswer: " << request.getTag();
        LOG(INFO) << "questioner: " << request.questioner;
        for (const auto& x : request.question) LOG(INFO) << "question: " << x;
        for (const auto& x : request.desc) LOG(INFO) << "desc: " << x.second;
        response->solution = {"solution1", "solution2"};
    });

    server.start();
    loop.loop();
    return 0;
}
