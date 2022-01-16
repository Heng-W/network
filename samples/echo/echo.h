#ifndef SAMPLES_ECHO_H
#define SAMPLES_ECHO_H

#include "net/tcp_server.h"

class EchoServer
{
public:
    EchoServer(net::EventLoop* loop, const net::InetAddress& listenAddr);

    void start() { server_.start(); }

private:
    net::TcpServer server_;
};

#endif // SAMPLES_ECHO_H
