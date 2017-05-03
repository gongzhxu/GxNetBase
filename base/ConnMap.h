#ifndef _CONN_MAP_H_
#define _CONN_MAP_H_

#include <assert.h>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

class BaseConn;
typedef std::shared_ptr<BaseConn> BaseConnPtr;

template<typename T>
class ConnMap
{
public:
    typedef std::map<T,  BaseConnPtr> ConnMap_t;
    typedef std::vector<BaseConnPtr> ConnList_t;

    ConnMap() {};
    ~ConnMap(){};

    BaseConnPtr getConn(const T & key)
    {
        BaseConnPtr pConn = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            auto it = connMap_.find(key);
            if(it != connMap_.end())
            {
                pConn = it->second;
            }
        }

        return pConn;
    }

    bool hasConn(const T & key)
    {
        std::unique_lock<std::mutex> lock(mutex_);
       auto it = connMap_.find(key);
        if(it != connMap_.end())
        {
            return true;
        }

        return false;
    }

    bool hasConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = connMap_.find(key);
        if(it != connMap_.end() && it->second == pConn)
        {
            return true;
        }

        return false;
    }

    void addConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connMap_[key] =  pConn;
    }

    void setConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connMap_[key] =  pConn;
    }

    void delConn(const T & key, const BaseConnPtr &)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connMap_.erase(key);
    }

    void delConn(const T & key)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connMap_.erase(key);
    }


    void stopConn()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = connMap_.begin(); it != connMap_.end(); ++it)
        {
            (it->second)->shutdown();
        }

        connMap_.clear();
    }

    void sendPdu(const std::shared_ptr<void> & pdu)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = connMap_.begin(); it != connMap_.end(); ++it)
        {
            (it->second)->sendPdu(pdu);
        }
    }

    size_t size() { return connMap_.size(); }

    void getAllConn(ConnList_t & connList)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = connMap_.begin(); it != connMap_.end(); ++it)
        {
            connList.emplace_back(it->second);
        }
    }
private:
    std::mutex mutex_;
    ConnMap_t connMap_;
};

#endif
