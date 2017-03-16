#include "ConnList.h"
#include "BaseConn.h"

BaseConnPtr ConnList::getNextConn()
{
    BaseConnPtr pConn = nullptr;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        size_t listSize = _connList.size();
        if(_next >= listSize)
        {
            _next = 0;
        }

        if(listSize > 0)
        {
           pConn = _connList[_next++];
        }
    }

    return pConn;
}

void ConnList::addConn(const BaseConnPtr & pConn)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _connList.emplace_back(pConn);
}

void ConnList::delConn(const BaseConnPtr & pConn)
{
    std::unique_lock<std::mutex> lock(_mutex);
    for(size_t i = 0; i < _connList.size(); ++i)
    {
        if(_connList[i] == pConn)
        {
            _connList.erase(_connList.begin() + i);
            break;
        }
    }
}
