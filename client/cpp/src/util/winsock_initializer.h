#ifndef WINSOCK_INITIALIZER_H
#define WINSOCK_INITIALIZER_H

#ifdef _WIN32

namespace detail
{

class WinsockInitializer
{
public:
    static WinsockInitializer& instance()
    {
        static WinsockInitializer initializer;
        return initializer;
    }
private:
    WinsockInitializer();
    ~WinsockInitializer();
};

} // namespace detail

#define INIT_WINSOCK_LIBRARY() \
    static ::detail::WinsockInitializer& s_detail_winsockInitializer = \
            ::detail::WinsockInitializer::instance()


#endif // _WIN32

#endif // WINSOCK_INITIALIZER_H