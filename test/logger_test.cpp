
#include "util/logger.h"

int main()
{
    util::Logger::setMinLogLevel(util::Logger::ERROR);

    LOG(INFO) << "aa";
    LOG(WARN) << "bb";
    //VLOG(INFO) << "aa";
    //VLOG(WARN) << "bb";
    DLOG(INFO) << "cc";
    SYSLOG(ERROR) << "err";
    LOG(ERROR) << "err2";

    int a = 20;
    //当条件满足时输出日志
    LOG_IF(INFO, a > 10) << "hhh";
    LOG_IF(WARN, a > 10) << "hhh";

    //google::COUNTER 记录该语句被执行次数，从1开始，在第一次运行输出日志之后，每隔 10 次再输出一次日志信息
    //LOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th cookie";

    //上述两者的结合，不过要注意，是先每隔 10 次去判断条件是否满足，
    //如果滞则输出日志；而不是当满足某条件的情况下，每隔 10 次输出一次日志信息
    //LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << google::COUNTER << "th big cookie";

    //当此语句执行的前 20 次都输出日志，然后不再输出
    //LOG_FIRST_N(INFO, 20) << "Got the " << google::COUNTER << "th cookie";
}
