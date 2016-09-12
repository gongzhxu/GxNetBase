#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <stdint.h>
#include <map>
#include <mutex>
#include <string>
#include <memory>

#include <event2/listener.h>

#include "ConnInfo.h"

class TcpServer;
class EventLoop;
class BaseConn;

typedef std::shared_ptr<BaseConn> BaseConnPtr;
typedef std::map<uint32_t,  BaseConnPtr> ConnMap_t;
typedef std::function<void (TcpServer *, evutil_socket_t)> AcceptCallback;
typedef std::shared_ptr<TcpServer> TcpServerPtr;

#define MakeTcpServerPtr std::make_shared<TcpServer>

class TcpServer
{
public:
    TcpServer(EventLoop * loop, const ConnInfo & ConnInfo, const AcceptCallback & cb);
    ~TcpServer();

    void start();
    void stop();
    ConnInfo & getConnInfo() { return _connInfo; }
private:
    void onConnect(const BaseConnPtr & pConn);
    void onClose(const BaseConnPtr & pConn);
    void onAccept(evutil_socket_t sockfd);


    static void onAccept(struct evconnlistener *,
                        evutil_socket_t sockfd,
                        struct sockaddr * addr,
                        int socklen,
                        void * arg);


private:
    EventLoop * _loop;
    ConnInfo _connInfo;
    struct evconnlistener * _listener;

    AcceptCallback _accept_cb;

    friend BaseConn;
};

#endif
