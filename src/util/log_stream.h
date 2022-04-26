#ifndef UTIL_LOG_STREAM_H
#define UTIL_LOG_STREAM_H

#include <iostream>

namespace util
{

class LogStreamBuf : public std::streambuf
{
public:
    // "len" must be >= 2 to account for the '\n' and '\0'.
    LogStreamBuf()
    {
        thread_local char buf[20480];
        setp(buf, buf + sizeof(buf) - 2);
    }

    // This effectively ignores overflow.
    int_type overflow(int_type ch) override { return ch; }

    // Legacy public ostrstream method.
    size_t pcount() const { return pptr() - pbase(); }
    char* pbase() const { return std::streambuf::pbase(); }
};

class LogStream : public std::ostream
{
public:
    LogStream()
        : std::ostream(nullptr),
          streambuf_(),
          ctr_(0),
          self_(this)
    { rdbuf(&streambuf_); }

    int ctr() const { return ctr_; }
    void set_ctr(int ctr) { ctr_ = ctr; }
    LogStream* self() const { return self_; }

    // Legacy std::streambuf methods.
    size_t pcount() const { return streambuf_.pcount(); }
    char* pbase() const { return streambuf_.pbase(); }
    char* str() const { return pbase(); }

private:
    LogStream(const LogStream&);
    LogStream& operator=(const LogStream&);
    LogStreamBuf streambuf_;
    int ctr_;
    LogStream* self_;
};

} // namespace util

#endif // UTIL_LOG_STREAM_H
