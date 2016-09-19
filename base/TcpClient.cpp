#include "TcpClient.h"

#include <assert.h>

#include "BaseUtil.h"
#include "BaseConn.h"
#include "EventLoop.h"
#include "SocketOps.h"

#define MIN_RETRY_INTERVAL 1
#define MAX_RETRY_INTERVAL 30

TcpClient::TcpClient(EventLoop * loop, const ConnectCallback & cb):
    _loop(loop),
    _connect_cb(cb)
{
    assert(_loop != nullptr);
}

TcpClient::~TcpClient()
{

}

void TcpClient::addClient(const ConnInfo & ci)
{
    _loop->runInLoop(std::bind(&TcpClient::addClientInLoop, this, ci));
}

void TcpClient::delClient(const ConnInfo & ci)
{
    _loop->runInLoop(std::bind(&TcpClient::delClientInLoop, this, ci));
}

BaseConnPtr TcpClient::getConn(const ConnInfo & ci)
{
    return _connMap.getConn(ci);
}

BaseConnPtr TcpClient::getNextConn()
{
#if 0
    BaseConnPtr pConn = _connList.getNextConn();
    if(!pConn)
    {
        LOG_INFO("get nullptr Conn");
    }
    else
    {
        LOG_INFO("get Conn:%p", pConn.get());
    }
    return pConn;
#else
    return _connList.getNextConn();
#endif // 1
}

void TcpClient::addClientInLoop(const ConnInfo & ci)
{
    LOG_INFO("addClientInLoop:%s, %d", ci.hostname().c_str(),  ci.id());
    if(!_connMap.hasConn(ci))
    {
        _connMap.addConn(ci, nullptr);
        _connect_cb(this, ci);
    }
    else
    {
        LOG_INFO("FindClientInLoop:%s, %d, %d", ci.hostname().c_str(),  ci.id());
    }
}

void TcpClient::delClientInLoop(const ConnInfo & ci)
{
     BaseConnPtr pConn = _connMap.getConn(ci);
     if(pConn)
     {
         _connList.delConn(pConn);
         pConn->shutdown();
     }

     _connMap.delConn(ci);
}

void TcpClient::onConnect(const BaseConnPtr & pConn)
{
    if(_connMap.hasConn(pConn->getConnInfo()))
    {
        _connList.addConn(pConn);
        BaseConnPtr pOldConn = _connMap.getConn(pConn->getConnInfo());
        if(pOldConn && !pOldConn->shutdownd())
        {
            pConn->shutdown();
            LOG_FATAL("not I want, something must be wrong");
        }
        else
        {
            base::setReuseAddr(pConn->getSockfd(), true);
            base::setReusePort(pConn->getSockfd(), true);
            base::setTcpNoDely(pConn->getSockfd(), true);
#if 1
            base::setKeepAlive(pConn->getSockfd(), true);
#endif // 0
            _connMap.setConn(pConn->getConnInfo(), pConn);
        }
    }
    else
    {
        pConn->shutdown();
    }
}

void TcpClient::onClose(const BaseConnPtr & pConn)
{
    _connList.delConn(pConn);

    if(pConn->shutdownd())
    {
        _connMap.delConn(pConn->getConnInfo());
        return;
    }

#if 1
    const ConnInfo & ci = pConn->getConnInfo();
    int interval = ci.retry();
    interval = interval < MIN_RETRY_INTERVAL? MIN_RETRY_INTERVAL: interval;
    interval = interval > MAX_RETRY_INTERVAL? MAX_RETRY_INTERVAL: interval;

    struct timeval tv= {interval, 0};
#else
    struct timeval tv= {0, 10};
#endif
    _loop->runAfter(tv, std::bind(&TcpClient::onRetry, this, pConn));
}

void TcpClient::onRetry(const BaseConnPtr & pConn)
{
    if(!pConn->shutdownd() && _connMap.hasConn(pConn->getConnInfo()))
    {
        _connMap.setConn(pConn->getConnInfo(), nullptr);
        _connect_cb(this, pConn->getConnInfo());
    }
}
