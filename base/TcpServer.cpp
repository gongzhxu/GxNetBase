#include "TcpServer.h"

#include <assert.h>

#include "BaseUtil.h"
#include "SocketOps.h"
#include "BaseConn.h"
#include "EventLoop.h"

TcpServer::TcpServer(EventLoop * loop, const ConnInfo & ci, const AcceptCallback & cb):
    _loop(loop),
    _connInfo(ci),
    _listener(nullptr),
    _accept_cb(cb)
{
    assert(_loop != nullptr);
}

TcpServer::~TcpServer()
{
    stop();
}

void TcpServer::start()
{
    assert(_listener == nullptr);
    struct sockaddr_in listenAddr;
    base::setAddr(_connInfo.host().c_str(), _connInfo.port(), &listenAddr);

    _listener = evconnlistener_new_bind(_loop->get_event(), onAccept, this,
                    LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
                    (struct sockaddr*)&listenAddr, sizeof(listenAddr));

    if(_listener == nullptr)
    {
        LOG_INFO("TcpServer listen port=%d, %s", _connInfo.port(), strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void TcpServer::stop()
{
    if(_listener)
    {
        evconnlistener_free(_listener);
        _listener = nullptr;
    }
}


void TcpServer::onConnect(const BaseConnPtr &)
{
    //LOG_TRACE("TcpServer::onConnect:%p", pConn.get());
}


void TcpServer::onClose(const BaseConnPtr &)
{
    //LOG_TRACE("TcpServer::onClose:%p", pConn.get());
}

void TcpServer::onAccept(evutil_socket_t sockfd)
{
    assert(_accept_cb);
    base::setReuseAddr(sockfd, true);
    base::setReusePort(sockfd, true);
    base::setTcpNoDely(sockfd, true);
#if 1
    base::setKeepAlive(sockfd, true);
#endif // 0
    //pass 'this' not safe
    _accept_cb(this, sockfd);
}

void TcpServer::onAccept(struct evconnlistener *,
                        evutil_socket_t sockfd,
                        struct sockaddr * ,
                        int ,
                        void * arg)
{
    static_cast<TcpServer *>(arg)->onAccept(sockfd);
}
