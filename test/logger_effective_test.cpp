
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>
#include <iostream>
#include "util/logger.h"
using namespace std;
using namespace std::chrono;

int main()
{
    //util::Logger::setLogLevel(util::Logger::WARNING);

    int fd = open("/dev/null", O_WRONLY);
    util::Logger::setOutput([fd](const char* msg, int len)
    {
        size_t n = ::write(fd, msg, len);
        (void)n;
    });

    auto t0 = steady_clock::now();


    for (int i = 0; i < 1000000; ++i)
    {
        LOG(INFO) << i;
    }

    auto t1 = steady_clock::now();
    cout << "runtime: " << duration_cast<milliseconds>(t1 - t0).count() << " ms" << endl;

    close(fd);

    //google::COUNTER 记录该语句被执行次数，从1开始，在第一次运行输出日志之后，每隔 10 次再输出一次日志信息
    //LOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th cookie";

    //上述两者的结合，不过要注意，是先每隔 10 次去判断条件是否满足，
    //如果滞则输出日志；而不是当满足某条件的情况下，每隔 10 次输出一次日志信息
    //LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << google::COUNTER << "th big cookie";

    //当此语句执行的前 20 次都输出日志，然后不再输出
    //LOG_FIRST_N(INFO, 20) << "Got the " << google::COUNTER << "th cookie";
}