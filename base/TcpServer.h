#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <stdint.h>
#include <set>
#include <map>
#include <mutex>
#include <string>
#include <memory>

#include <event2/listener.h>

#include "ConnInfo.h"
#include "SocketOps.h"
#include "BaseConn.h"
#include "EventLoop.h"

class TcpServer;
typedef std::shared_ptr<TcpServer> TcpServerPtr;
#define MakeTcpServerPtr std::make_shared<TcpServer>

class TcpServer
{
public:
    typedef std::map<ConnInfo, struct evconnlistener *> ListenMap_t;

    TcpServer(EventLoop * loop);
    ~TcpServer();

    template<typename T>
    void addServer(ConnInfo & ci)
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _connList.insert(ci);
        }
        _loop->runInLoop(std::bind(&TcpServer::addServerInLoop<T>, this, ci));
    }

    void delServer(ConnInfo & ci);

    void getConnInfo(std::vector<ConnInfo> & connList);
private:
    template<typename T>
    void addServerInLoop(ConnInfo & ci)
    {
        struct evconnlistener * listener = _listeners[ci];
        if(!listener)
        {
            sockaddr_storage sockAddr;
            int sockLen = base::makeAddr(ci.getCurrAddrInfo(), sockAddr);

            _listeners[ci] = evconnlistener_new_bind(_loop->get_event(), onAccept<T>, this,
                            LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
                            (const sockaddr *)&sockAddr, sockLen);
        }
    }

    void delServerInLoop(ConnInfo & ci);

    template<typename T>
    void onAccept(ConnInfo & ci)
    {
        base::setTcpNoDely(ci.fd(), true);
        base::setKeepAlive(ci.fd(), true);

        BaseConnPtr  pConn(new T);
        pConn->setConnectCallback(std::bind(&TcpServer::onConnect, this, pConn));
        pConn->setCloseCallback(std::bind(&TcpServer::onClose, this, pConn));
        pConn->doAccept(ci);
    }

    template<typename T>
    static void onAccept(struct evconnlistener *,
                        evutil_socket_t sockfd,
                        struct sockaddr * sockAddr,
                        int sockLen,
                        void * arg)
    {
        ConnInfo ci(sockfd);
        ci.addAddrInfo(base::getAddr(sockAddr, sockLen));
        static_cast<TcpServer *>(arg)->onAccept<T>(ci);
    }

    void onConnect(const BaseConnPtr & pConn);
    void onClose(const BaseConnPtr & pConn);
    void onMessage(const BaseConnPtr & pConn);
private:
    EventLoop *             _loop;
    struct evconnlistener * _listener;

    std::mutex               _mutex;
    std::set<ConnInfo>      _connList;

    ListenMap_t              _listeners;
};

#endif
