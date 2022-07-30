
#include "key.msg.h"

#include "util/logger.h"
#include "net/event_loop.h"
#include "net/tcp_server.h"
#include "net_ext/message/message_ssl_codec.h"
#include "net_ext/ssl/rsa.h"

static std::string s_privateKey;

namespace net
{

static void handleAESKey(const net::TcpConnectionPtr& conn,
                         const std::shared_ptr<EncryptedAESKey>& message,
                         util::Timestamp)
{
    std::string aesKey = ssl::rsa::privateDecrypt(message->key, s_privateKey); // 私钥解密
    conn->setContext(std::move(aesKey));
}

static void handleText(const net::TcpConnectionPtr& conn,
                       const std::shared_ptr<Text>& message,
                       util::Timestamp)
{
    LOG(INFO) << message->what;
}

REGISTER_MESSAGE_HANDLER(EncryptedAESKey, handleAESKey)
REGISTER_MESSAGE_HANDLER(Text, handleText)

} // namespace test

int main()
{
    using namespace net;
    auto keyPair = ssl::rsa::generateKey();
    s_privateKey = keyPair.first;
    std::string publicKey = keyPair.second;
    EventLoop loop;
    TcpServer server(&loop, InetAddress(18825));
    server.setConnectionCallback([&publicKey](const TcpConnectionPtr & conn)
    {
        if (conn->connected())
        {
            ServerRSAKey msg;
            msg.key = publicKey;
            ssl::send(conn, msg);
        }
    });
    server.setMessageCallback(&ssl::codec::onMessage);
    server.start();
    loop.loop();
    return 0;
}
