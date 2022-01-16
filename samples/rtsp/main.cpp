
#include <unistd.h>
#include <thread>

#include "util/logger.h"
#include "net/event_loop.h"
#include "net_ext/rtsp/rtsp_server.h"

int main()
{
    LOG(INFO) << "pid = " << getpid();
    LOG(INFO) << "tid = " << std::this_thread::get_id();
    net::EventLoop loop;
    net::RtspServer server(&loop);
    server.start();
    loop.loop();
}

