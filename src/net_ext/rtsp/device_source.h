#ifndef NET_DEVICE_SOURCE_H
#define NET_DEVICE_SOURCE_H

#include <set>
#include <vector>
#include <memory>
#include <mutex>

namespace net
{

class Buffer;
class MediaSession;
class H264Source;

class DeviceSource
{
public:
    DeviceSource();
    ~DeviceSource();

    void addSession(const std::shared_ptr<MediaSession>& session);

    bool handleFrame(Buffer* frame, int frameSize);

private:
    bool send(Buffer* frame, int frameSize);

    std::unique_ptr<H264Source> source_;
    std::set<std::weak_ptr<MediaSession>, std::owner_less<std::weak_ptr<MediaSession>>> sessions_;
    std::vector<std::weak_ptr<MediaSession>> newSessions_;
    mutable std::mutex mutex_;
};

} // namespace net

#endif // NET_DEVICE_SOURCE_H
