
#include "key.msg.h"

#include "util/logger.h"
#include "net/event_loop.h"
#include "net/tcp_client.h"
#include "net_ext/message/message_ssl_codec.h"
#include "net_ext/ssl/rsa.h"
#include "net_ext/ssl/aes.h"

namespace net
{

static void handleServerRSAKey(const net::TcpConnectionPtr& conn,
                               const std::shared_ptr<ServerRSAKey>& message,
                               util::Timestamp)
{
    std::string aesKey = ssl::generateAESKey(); // 生成随机对称密钥
    std::string encoded = ssl::rsa::publicEncrypt(aesKey, message->key); // 用Server公钥加密
    EncryptedAESKey resp;
    resp.key = encoded;
    ssl::send(conn, resp);
    conn->setContext(std::move(aesKey));
    Text text;
    text.what = "hello";
    ssl::send(conn, text);
    conn->shutdown();
}

REGISTER_MESSAGE_HANDLER(ServerRSAKey, handleServerRSAKey)

} // namespace net


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
    });

    client.setMessageCallback(&ssl::codec::onMessage);
    client.connect();
    loop.loop();
    return 0;
}
