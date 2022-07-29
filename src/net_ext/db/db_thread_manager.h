#ifndef NET_DB_THREAD_MANAGER_H
#define NET_DB_THREAD_MANAGER_H

#include <functional>
#include <vector>
#include <thread>
#include "util/common.h"
#include "util/blocking_queue.hpp"
#include "db_connection.h"

namespace net
{

using DBTask = std::function<void(DBConnection*)>;

class DBThread
{
public:

    DBThread(const DBInfo& info)
        : quit_(false),
          dbConnection_(info),
          thread_(&DBThread::threadFunc, this) {}

    ~DBThread()
    {
        quit_ = true;
        dbTasks_.put(nullptr);
        thread_.join();
    }

    DISALLOW_COPY_AND_ASSIGN(DBThread);

    void addTask(const DBTask& task) { dbTasks_.put(task); }

    void threadFunc()
    {
        while (!quit_)
        {
            DBTask cb = dbTasks_.get();
            if (cb) cb(&dbConnection_);
        }
    }

private:
    bool quit_;
    DBConnection dbConnection_;
    std::thread thread_;
    util::BlockingQueue<DBTask> dbTasks_;
};


class DBThreadManager
{
public:

    DBThreadManager(const DBInfo& info);

    DISALLOW_COPY_AND_ASSIGN(DBThreadManager);

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    void start();
    bool started() const { return started_; }

    void addTask(const DBTask& task);

private:
    std::vector<std::unique_ptr<DBThread>> dbThreads_;
    DBInfo dbInfo_;
    bool started_;
    int numThreads_;
    int index_;
};

} // namespace net

#endif // NET_DB_THREAD_MANAGER_H
