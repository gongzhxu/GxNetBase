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
        std::unique_lock<std::mutex> lock(mutex_);
        connsMap_[key].insert(pConn);
    }

    void delConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connsMap_[key].erase(pConn);
    }

    void stopConn()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = connsMap_.begin(); it != connsMap_.end(); ++it)
        {
            auto key = it->first;
            for(auto it1 = connsMap_[key].begin(); it1 != connsMap_[key].end(); ++it1)
            {
                (*it1)->shutdown();
            }
        }

        connsMap_.clear();
    }

    void sendPdu(const T & key, const std::shared_ptr<void> & pdu)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = connsMap_[key].begin(); it != connsMap_[key].end(); ++it)
        {
            (*it)->sendPdu(pdu);
        }
    }

    size_t size()
    {
        return connsMap_.size();
    }

    size_t size(const T & key)
    {
        size_t num = 0;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            num = connsMap_[key].size();
        }

        return num;
    }

    void getAllKey(std::vector<T> & keyList)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = connsMap_.begin(); it != connsMap_.end(); ++it)
        {
            keyList.emplace_back(it->first);
        }
    }

    void getAllConn(const T & key, ConnList_t & connList)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = connsMap_[key].begin(); it != connsMap_[key].end(); ++it)
        {
            connList.emplace_back(*it);
        }
    }
private:
    std::mutex mutex_;
    ConnsMap_t connsMap_;
};

#endif // _CONNS_MAP_H_
