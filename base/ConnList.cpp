#include "ConnList.h"
#include "BaseConn.h"

BaseConnPtr ConnList::getNextConn()
{
    BaseConnPtr pConn = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        size_t listSize = connList_.size();
        if(next_ >= listSize)
        {
            next_ = 0;
        }

        if(listSize > 0)
        {
           pConn = connList_[next_++];
        }
    }

    return pConn;
}

void ConnList::addConn(const BaseConnPtr & pConn)
{
    std::unique_lock<std::mutex> lock(mutex_);
    connList_.emplace_back(pConn);
}

void ConnList::delConn(const BaseConnPtr & pConn)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for(size_t i = 0; i < connList_.size(); ++i)
    {
        if(connList_[i] == pConn)
        {
            connList_.erase(connList_.begin() + i);
            break;
        }
    }
}
