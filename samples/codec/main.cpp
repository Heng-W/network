
#include "time.h"

#include "util/logger.h"
#include "net/event_loop.h"

#include <unistd.h>
#include <thread>

int main()
{
    LOG(INFO) << "pid = " << getpid();
    LOG(INFO) << "tid = " << std::this_thread::get_id();
    net::EventLoop loop;
    net::InetAddress listenAddr(2037);
    TimeServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}

