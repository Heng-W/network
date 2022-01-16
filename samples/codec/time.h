#ifndef SAMPLES_TIME_H
#define SAMPLES_TIME_H

#include "net/tcp_server.h"

class TimeServer
{
public:
    TimeServer(net::EventLoop* loop, const net::InetAddress& listenAddr);

    void start() { server_.start(); }

private:
    net::TcpServer server_;
};

#endif // SAMPLES_TIME_H
