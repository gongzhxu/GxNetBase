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
    void addClient(ConnInfo & ci)
    {
        loop_->runInLoop(std::bind(&TcpClient::addClientInLoop<T>, this, ci));
    }

    template<typename T, typename D>
    void addClient(ConnInfo & ci, D * d)
    {
        loop_->runInLoop(std::bind(&TcpClient::addClientInLoop<T,D>, this, ci, d));
    }


    void delClient(ConnInfo & ci);

    BaseConnPtr getConn(ConnInfo & ci);
    BaseConnPtr getNextConn(int type = 0);

    size_t size() { return connList_.size(); }
private:
    template<typename T>
    void addClientInLoop(ConnInfo & ci)
    {
        if(!connMap_.hasConn(ci))
        {
            connMap_.addConn(ci, nullptr);
            BaseConnPtr  pConn(new T);
            pConn->setConnectCallback(std::bind(&TcpClient::onConnect, this, pConn));
            pConn->setCloseCallback(std::bind(&TcpClient::onClose, this, pConn));
            pConn->doConnect(ci);
        }
    }

    template<typename T, typename D>
    void addClientInLoop(ConnInfo & ci, D * d)
    {
        if(!connMap_.hasConn(ci))
        {
            connMap_.addConn(ci, nullptr);
            BaseConnPtr  pConn(new T(d));
            pConn->setConnectCallback(std::bind(&TcpClient::onConnect, this, pConn));
            pConn->setCloseCallback(std::bind(&TcpClient::onClose, this, pConn));
            pConn->doConnect(ci);
        }
    }

    void delClientInLoop(ConnInfo & ci);

    void onConnect(const BaseConnPtr & pConn);
    void onClose(const BaseConnPtr & pConn);
    void onRetry(const BaseConnPtr & pConn);

private:
    EventLoop *              loop_;
    std::map<int, ConnList>  connList_;
    ConnMap<ConnInfo>       connMap_;

    friend BaseConn;
};

#endif
