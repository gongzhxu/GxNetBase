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
    void addServer(const ConnInfo & ci)
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _connList.insert(ci);
        }
        _loop->runInLoop(std::bind(&TcpServer::addServerInLoop<T>, this, ci));
    }

    void delServer(const ConnInfo & ci);

    void getConnInfo(std::vector<ConnInfo> & connList);
private:
    template<typename T>
    void addServerInLoop(const ConnInfo & ci)
    {
        struct evconnlistener * listener = _listeners[ci];
        if(!listener)
        {
            AddrInfo addrInfo = ci.getCurrAddrInfo();

            base::SockAddr sockAddr;
            int sockLen = base::setAddr(addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port(), &sockAddr);

            _listeners[ci] = evconnlistener_new_bind(_loop->get_event(), onAccept<T>, this,
                            LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
                            (const sockaddr *)&sockAddr, sockLen);
        }
    }

    void delServerInLoop(const ConnInfo & ci);

    template<typename T>
    void onAccept(evutil_socket_t sockfd)
    {
        base::setReuseAddr(sockfd, true);
        base::setReusePort(sockfd, true);
        base::setTcpNoDely(sockfd, true);
        base::setKeepAlive(sockfd, true);
        BaseConnPtr  pConn(new T);
        pConn->doAccept(this, sockfd);
    }

    template<typename T>
    static void onAccept(struct evconnlistener *,
                        evutil_socket_t sockfd,
                        struct sockaddr *,
                        int,
                        void * arg)
    {
        static_cast<TcpServer *>(arg)->onAccept<T>(sockfd);
    }

    void onConnect(const BaseConnPtr & pConn);
    void onClose(const BaseConnPtr & pConn);
private:
    EventLoop *             _loop;
    struct evconnlistener * _listener;

    std::mutex               _mutex;
    std::set<ConnInfo>      _connList;

    ListenMap_t              _listeners;

    friend BaseConn;
};

#endif
