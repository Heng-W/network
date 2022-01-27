
#include "query.msg.h"

#include "util/logger.h"
#include "net/event_loop.h"
#include "net/tcp_client.h"
#include "net_ext/message/message_codec.h"
#include "net_ext/rpc/rpc_client_service.h"


int main()
{
    using namespace net;
    EventLoop loop;
    TcpClient client(&loop, InetAddress(18825));
    client.setMessageCallback(&codec::onMessage);
    registerRpcResponseHandler();

    client.setConnectionCallback([&loop](const TcpConnectionPtr & conn)
    {
        if (!conn->connected())
        {
            loop.quit();
        }
        else
        {
            test::Query query;
            query.questioner = "hw";
            query.question = {"question1", "question2"};
            query.desc = {{1, "1"}, {2, "2"}};
            rpcSolve<test::Answer>(conn, query, [conn](const std::shared_ptr<test::Answer>& resp)
            {
                LOG(INFO) << "get: " << resp->getTag();
                for (const auto& x : resp->solution) LOG(INFO) << "solution: " << x;
                conn->shutdown();
            });
        }
    });

    client.connect();
    loop.loop();
    return 0;
}
