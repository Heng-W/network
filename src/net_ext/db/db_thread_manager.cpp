
#include "db_thread_manager.h"
#include <assert.h>

namespace net
{

DBThreadManager::DBThreadManager(const DBInfo& info)
    : dbInfo_(info),
      started_(false),
      numThreads_(0),
      index_(0)
{
}

void DBThreadManager::start()
{
    assert(!started_);
    assert(numThreads_ >= 0);

    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        dbThreads_.emplace_back(new DBThread(dbInfo_));
    }
}

void DBThreadManager::addTask(const DBTask& dbTask)
{
    if (index_ >= numThreads_)
    {
        index_ = 0;
    }
    dbThreads_[index_++]->addTask(dbTask);
}

} // namespace net
