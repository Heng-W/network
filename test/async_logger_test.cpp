
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "util/async_logger.h"
#include "util/logger.h"
#include "util/timestamp.h"

using std::string;

off_t kRollSize = 500 * 1000 * 1000;

util::AsyncLogger* g_asyncLog = NULL;

void bench(bool longLog)
{
    util::Logger::setOutput([](const char* msg, int len)
    {
        g_asyncLog->append(msg, len);
    });

    int cnt = 0;
    const int kBatch = 1000;
    string empty = " ";
    string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; ++t)
    {
        auto start = util::Timestamp::now();
        for (int i = 0; i < kBatch; ++i)
        {
            LOG(INFO) << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                      << (longLog ? longStr : empty)
                      << cnt;
            ++cnt;
        }
        std::cout << (util::Timestamp::now() - start).toMsec() << std::endl;
        struct timespec ts = { 0, 500 * 1000 * 1000 };
        nanosleep(&ts, NULL);
    }
}

int main(int argc, char* argv[])
{
    printf("pid = %d\n", getpid());

    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof name - 1);
    util::AsyncLogger log(::basename(name), kRollSize);
    log.start();
    g_asyncLog = &log;

    bool longLog = argc > 1;
    bench(longLog);
    return 0;
}
