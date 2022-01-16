#ifndef UTIL_BLOCKING_QUEUE_HPP
#define UTIL_BLOCKING_QUEUE_HPP

#include <deque>
#include <condition_variable>

namespace util
{

template <class T>
class BlockingQueue
{
public:
    using QueueType = std::deque<T>;

    void put(const T& x)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify_one();
    }

    void put(T&& x)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(x));
        notEmpty_.notify_one();
    }

    T get()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, [this] { return !queue_.empty(); });
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return front;
    }

    int size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    QueueType queue_;
};

} // namespace util

#endif // UTIL_BLOCKING_QUEUE_HPP
