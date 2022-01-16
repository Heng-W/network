#ifndef NET_RTSP_SERVER_INFO_H
#define NET_RTSP_SERVER_INFO_H

#include <string>
#include <map>

namespace net
{

class DeviceSource;
class Socket;

struct RtspServerInfo
{

    void addDeviceSource(const std::string& session, DeviceSource* source)
    { sources.insert({session, source}); }

    DeviceSource* findSource(const std::string& session)
    {
        auto it = sources.find(session);
        return it != sources.end() ? it->second : nullptr;
    }

    std::map<std::string, DeviceSource*> sources;

    // for udp
    std::unique_ptr<Socket> rtpSocket;
    std::unique_ptr<Socket> rtcpSocket;

    uint16_t rtpPort;
    uint16_t rtcpPort;
};

} // namespace net

#endif // NET_RTSP_SERVER_INFO_H
