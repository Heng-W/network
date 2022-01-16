#ifndef NET_HTTP_SERVER_H
#define NET_HTTP_SERVER_H

#include "net/tcp_server.h"

namespace net
{

class HttpRequest;
class HttpResponse;

// simple http server
class HttpServer
{
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               TcpServer::Option option = TcpServer::kNoReusePort);

    // 在调用start()前设置（非线程安全）
    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    EventLoop* getLoop() const { return server_.getLoop(); }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

    TcpServer server_;
    HttpCallback httpCallback_;
};

} // namespace net

#endif // NET_HTTP_SERVER_H

