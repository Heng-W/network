
#include "device_source.h"

#include "media_session.h"
#include "h264_source.h"

namespace net
{

DeviceSource::DeviceSource()
    : source_(new H264Source())
{
    source_->setSendFrameCallback([this](Buffer * frame, int frameSize)
    { return this->send(frame, frameSize); });
}

DeviceSource::~DeviceSource() = default;

void DeviceSource::addSession(const std::shared_ptr<MediaSession>& session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    newSessions_.push_back(session);
}

bool DeviceSource::handleFrame(Buffer* frame, int frameSize)
{
    decltype(newSessions_) newSessions;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        newSessions.swap(newSessions_);
    }
    for (auto& x : newSessions) sessions_.insert(std::move(x));
    for (auto& session : sessions_)
    {
        auto sessionGuard = session.lock();
        if (sessionGuard)
        {
            sessionGuard->updateTimestamp();
        }
    }
    return source_->handleFrame(frame, frameSize);
}


bool DeviceSource::send(Buffer* frame, int frameSize)
{
    for (auto& session : sessions_)
    {
        auto sessionGuard = session.lock();
        if (sessionGuard)
        {
            sessionGuard->send(frame, frameSize);
        }
        else
        {
            sessions_.erase(session);
        }
    }
    return true;
}

} // namespace net

