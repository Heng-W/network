
#include "query.msg.h"

#include "util/logger.h"
#include "net/event_loop.h"
#include "net/tcp_server.h"
#include "net_ext/message/message_codec.h"

namespace test
{

static void handleRequest(const net::TcpConnectionPtr& conn,
                          const std::shared_ptr<Query>& message,
                          util::Timestamp)
{
    // handle code
    LOG(INFO) << "onAnswer: " << message->getTag();
    LOG(INFO) << "questioner: " << message->questioner;
    for (const auto& x: message->question) LOG(INFO) << "question: " << x;
    for (const auto& x: message->desc) LOG(INFO) << "desc: " << x.second;
    Answer answer;
    answer.solution = {"solution1", "solution2"};
    send(conn, answer);
    conn->shutdown();
}

REGISTER_MESSAGE_HANDLER(Query, handleRequest)

} // namespace test

int main()
{
    using namespace net;
    EventLoop loop;
    TcpServer server(&loop, InetAddress(18825));
    server.setMessageCallback(&codec::onMessage);
    server.start();
    loop.loop();
    return 0;
}
