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

    ConnMap() {};
    ~ConnMap() {};

    BaseConnPtr getConn(const T & key)
    {
        BaseConnPtr pConn = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            typename ConnMap_t::iterator it = _connMap.find(key);
            if(it != _connMap.end())
            {
                pConn = it->second;
            }
        }

        return pConn;
    }

    bool hasConn(const T & key)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        typename ConnMap_t::iterator it = _connMap.find(key);
        if(it != _connMap.end())
        {
            return true;
        }

        return false;
    }

    bool hasConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        typename ConnMap_t::iterator it = _connMap.find(key);
        if(it != _connMap.end() && it->second == pConn)
        {
            return true;
        }

        return false;
    }

    void addConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //assert(_connMap[key] == nullptr);
        _connMap[key] =  pConn;
    }

    void setConn(const T & key, const BaseConnPtr & pConn)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _connMap[key] =  pConn;
    }

    void delConn(const T & key, const BaseConnPtr &)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //assert(_connMap[key] == nullptr || _connMap[key] == pConn);
        _connMap.erase(key);
    }

    void delConn(const T & key)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _connMap.erase(key);
    }


    void stopConn()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for(typename ConnMap_t::iterator it = _connMap.begin(); it != _connMap.end(); ++it)
        {
            (it->second)->shutdown();
        }

        _connMap.clear();
    }

    void sendPdu(const std::shared_ptr<void> & pdu)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for(typename ConnMap_t::iterator it = _connMap.begin(); it != _connMap.end(); ++it)
        {
            (it->second)->sendPdu(pdu);
        }
    }

    size_t size() { return _connMap.size(); }

    void getAllConn(std::vector<BaseConnPtr> & connList)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for(typename ConnMap_t::iterator it = _connMap.begin(); it != _connMap.end(); ++it)
        {
            connList.push_back(it->second);
        }
    }
private:
    std::mutex _mutex;
    ConnMap_t _connMap;
};

#endif
