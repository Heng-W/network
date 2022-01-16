
#include "http_server.h"

#include "util/logger.h"
#include "http_context.h"
#include "http_request.h"
#include "http_response.h"

namespace net
{

namespace
{

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

} // namespace

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       TcpServer::Option option)
    : server_(loop, listenAddr, option),
      httpCallback_(defaultHttpCallback)
{
    server_.setConnectionCallback([this](const TcpConnectionPtr & conn) { this->onConnection(conn); });
    server_.setMessageCallback(
        [this](const TcpConnectionPtr & conn, Buffer * buf, Timestamp receiveTime)
    {
        this->onMessage(conn, buf, receiveTime);
    });
}

void HttpServer::start()
{
    LOG(INFO) << "HttpServer[" << server_.name()
              << "] starts listening on " << server_.ipPort();
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    HttpContext* context = util::any_cast<HttpContext>(conn->getMutableContext());

    if (!context->parseRequest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll())
    {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                 (req.version() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(std::move(buf));
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}

} // namespace net
