
#include "query.msg.h"

#include "util/logger.h"
#include "net/event_loop.h"
#include "net/tcp_client.h"
#include "net_ext/message/message_codec.h"

namespace test
{

static void handleResponse(const net::TcpConnectionPtr& conn,
                           const std::shared_ptr<Answer>& message,
                           util::Timestamp)
{
    // handle code
    LOG(INFO) << "get: " << message->getTag();
    for (const auto& x: message->solution) LOG(INFO) << "solution: " << x;
}

REGISTER_MESSAGE_HANDLER(Answer, handleResponse)

} // namespace test


int main()
{
    using namespace net;
    EventLoop loop;
    TcpClient client(&loop, InetAddress(18825));
    client.setConnectionCallback([&loop](const TcpConnectionPtr & conn)
    {
        LOG(INFO) << conn->localAddr().toIpPort() << " -> "
                  << conn->peerAddr().toIpPort() << " is "
                  << (conn->connected() ? "UP" : "DOWN");

        if (!conn->connected())
        {
            loop.quit();
        }
        else
        {
            test::Query query;
            query.questioner = "hw";
            query.question = {"question1", "question2"};
            send(conn, query);
        }
    });


    client.setMessageCallback(&codec::onMessage);
    client.connect();
    loop.loop();
    return 0;
}
