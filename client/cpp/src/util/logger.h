#ifndef UTIL_LOGGER_H
#define UTIL_LOGGER_H

#include <functional>
#include <sstream> // ostringstream

namespace util
{

class Logger
{
public:
    using OutputFunc = std::function<void(const char* msg, int len)>;
    using FlushFunc = std::function<void()>;

    // 日志等级
    enum LogLevel {TRACE, DEBUG, INFO, WARN, ERROR, FATAL, OFF};

    Logger(const char* file, int line, LogLevel level, bool sysLog = false);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();

    static std::ostream& stream() { return stream_; }

    static LogLevel minLogLevel() { return minLogLevel_; }
    static void setMinLogLevel(LogLevel level) { minLogLevel_ = level; }
    static void setOutput(const OutputFunc& output) { output_ = output; }
    static void setFlush(const FlushFunc& flush) { flush_ = flush; }

private:
    Logger(LogLevel level, int savedErrno, const char* file, int line);

    static LogLevel minLogLevel_;
    static OutputFunc output_;
    static FlushFunc flush_;
    static thread_local std::ostringstream stream_;

    LogLevel level_;
};


// 参考glog实现，此类用于显式忽略条件日志宏中的值
// 避免如“未使用计算值”、“语句无效”之类的编译器警告
struct LogMessageVoidify
{
    // 使用的运算符优先级应低于<<但高于？:
    void operator&(std::ostream&) {}
};

// 内部使用的宏
#define _LOG_TRACE util::Logger(__FILE__, __LINE__, util::Logger::TRACE, __func__)
#define _LOG_DEBUG util::Logger(__FILE__, __LINE__, util::Logger::DEBUG, __func__)
#define _LOG_INFO util::Logger(__FILE__, __LINE__, util::Logger::INFO)
#define _LOG_WARN util::Logger(__FILE__, __LINE__, util::Logger::WARN)
#define _LOG_ERROR util::Logger(__FILE__, __LINE__, util::Logger::ERROR)
#define _LOG_FATAL util::Logger(__FILE__, __LINE__, util::Logger::FATAL)

#define _LOG_SYS_ERROR util::Logger(__FILE__, __LINE__, util::Logger::ERROR, true)
#define _LOG_SYS_FATAL util::Logger(__FILE__, __LINE__, util::Logger::FATAL, true)

#define _LOG(level) _LOG_ ## level.stream()
#define _SYSLOG(level) _LOG_SYS_ ## level.stream()

#define _LOG_IF(level, condition) \
    !(condition) ? (void)0 : util::LogMessageVoidify() & _LOG(level)
#define _SYSLOG_IF(level, condition) \
    !(condition) ? (void)0 : util::LogMessageVoidify() & _SYSLOG(level)

// 用户宏
#define LOG_IS_ON(level) (util::Logger::level >= util::Logger::minLogLevel())

#define LOG(level) _LOG_IF(level, LOG_IS_ON(level))
#define LOG_IF(level, condition) _LOG_IF(level, (condition) && LOG_IS_ON(level))

#define SYSLOG(level) _SYSLOG_IF(level, LOG_IS_ON(level))
#define SYSLOG_IF(level, condition) _SYSLOG_IF(level, (condition) && LOG_IS_ON(level))

#ifndef NDEBUG
#define DLOG(level) LOG(level)
#define DLOG_IF(level, condition) LOG_IF(level, condition)
#else // release模式下不输出
#define DLOG(level) LOG_IF(level, false)
#define DLOG_IF(level, condition) LOG_IF(level, false)
#endif

// 检查输入非空
#define CHECK_NOTNULL(val) \
    ::util::checkNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
inline T* checkNotNull(const char* file, int line, const char* names, T* ptr)
{
    if (ptr == nullptr)
    {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}

} // namespace util

#endif // UTIL_LOGGER_H
