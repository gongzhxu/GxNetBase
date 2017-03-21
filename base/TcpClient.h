#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <stdint.h>
#include <vector>
#include <memory>

#include "ConnInfo.h"
#include "ConnList.h"
#include "ConnMap.h"
#include "BaseConn.h"
#include "EventLoop.h"

class TcpClient;
typedef std::shared_ptr<TcpClient> TcpClientPtr;
#define MakeTcpClientPtr std::make_shared<TcpClient>

class TcpClient
{
public:
    TcpClient(EventLoop * loop);
    ~TcpClient();

public:
    template<typename T>
    void addClient(const ConnInfo & ci)
    {
        _loop->runInLoop(std::bind(&TcpClient::addClientInLoop<T>, this, ci));
    }

    void delClient(const ConnInfo & ci);

    BaseConnPtr getConn(const ConnInfo & ci);
    BaseConnPtr getNextConn(int type = 0);

    size_t size() { return _connList.size(); }
private:
    template<typename T>
    void addClientInLoop(const ConnInfo & ci)
    {
        if(!_connMap.hasConn(ci))
        {
            _connMap.addConn(ci, nullptr);
            BaseConnPtr  pConn(new T);
            pConn->doConnect(this, ci);
        }
    }

    void delClientInLoop(const ConnInfo & ci);

    void onConnect(const BaseConnPtr & pConn);
    void onClose(const BaseConnPtr & pConn);
    void onRetry(const BaseConnPtr & pConn);

private:
    EventLoop *              _loop;
    std::map<int, ConnList>  _connList;
    ConnMap<ConnInfo>       _connMap;

    friend BaseConn;
};

#endif
