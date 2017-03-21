#ifndef _CONNS_MAP_H_
#define _CONNS_MAP_H_

#include <map>
#include <vector>
#include <set>
#include <mutex>
#include <memory>

class BaseConn;
typedef std::shared_ptr<BaseConn> BaseConnPtr;

template<typename T>
class ConnsMap
{
public:
    typedef std::map<T,  std::set<BaseConnPtr> > ConnsMap_t;
    typedef std::vector<BaseConnPtr> ConnList_t;

    ConnsMap() {};
    ~ConnsMap(){};

    void addConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _connsMap[key].insert(pConn);
    }

    void delConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _connsMap[key].erase(pConn);
    }

    void stopConn()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for(auto it = _connsMap.begin(); it != _connsMap.end(); ++it)
        {
            auto key = it->first;
            for(auto it1 = _connsMap[key].begin(); it1 != _connsMap[key].end(); ++it1)
            {
                (*it1)->shutdown();
            }
        }

        _connsMap.clear();
    }

    void sendPdu(const T & key, const std::shared_ptr<void> & pdu)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for(auto it = _connsMap[key].begin(); it != _connsMap[key].end(); ++it)
        {
            (*it)->sendPdu(pdu);
        }
    }

    size_t size()
    {
        return _connsMap.size();
    }

    size_t size(const T & key)
    {
        size_t num = 0;

        {
            std::unique_lock<std::mutex> lock(_mutex);
            num = _connsMap[key].size();
        }

        return num;
    }

    void getAllKey(std::vector<T> & keyList)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for(auto it = _connsMap.begin(); it != _connsMap.end(); ++it)
        {
            keyList.emplace_back(it->first);
        }
    }

    void getAllConn(const T & key, ConnList_t & connList)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for(auto it = _connsMap[key].begin(); it != _connsMap[key].end(); ++it)
        {
            connList.emplace_back(*it);
        }
    }
private:
    std::mutex _mutex;
    ConnsMap_t _connsMap;
};

#endif // _CONNS_MAP_H_
