#include "TcpClient.h"

#include <assert.h>

#include "BaseUtil.h"
#include "SocketOps.h"

TcpClient::TcpClient(EventLoop * loop):
    _loop(loop)
{
    assert(_loop != nullptr);
}


TcpClient::~TcpClient()
{

}

void TcpClient::delClient(ConnInfo & ci)
{
    _loop->runInLoop(std::bind(&TcpClient::delClientInLoop, this, ci));
}

BaseConnPtr TcpClient::getConn(ConnInfo & ci)
{
    return _connMap.getConn(ci);
}

BaseConnPtr TcpClient::getNextConn(int type)
{
    return _connList[type].getNextConn();
}

void TcpClient::delClientInLoop(ConnInfo & ci)
{
     BaseConnPtr pConn = _connMap.getConn(ci);
     if(pConn)
     {
         _connList[ci.type()].delConn(pConn);
         pConn->shutdown();
     }

     _connMap.delConn(ci);
}

void TcpClient::onConnect(const BaseConnPtr & pConn)
{
    const ConnInfo & ci = pConn->getConnInfo();
    if(_connMap.hasConn(ci))
    {
        _connList[ci.type()].addConn(pConn);
        BaseConnPtr pOldConn = _connMap.getConn(ci);
        if(pOldConn && !pOldConn->shutdownd())
        {
            pConn->shutdown();
        }
        else
        {
            base::setReuseAddr(ci.fd(), true);
            base::setReusePort(ci.fd(), true);
            base::setTcpNoDely(ci.fd(), true);
            base::setKeepAlive(ci.fd(), true);
            _connMap.setConn(ci, pConn);
        }
    }
    else
    {
        pConn->shutdown();
    }
}

void TcpClient::onClose(const BaseConnPtr & pConn)
{
    const ConnInfo & ci = pConn->getConnInfo();

    _connList[ci.type()].delConn(pConn);

    if(pConn->shutdownd())
    {
        _connMap.delConn(ci);
        return;
    }

    struct timeval tv= {ci.retry(), 0};
    _loop->runAfter(tv, std::bind(&TcpClient::onRetry, this, pConn));
}

void TcpClient::onRetry(const BaseConnPtr & pConn)
{
    if(!pConn->shutdownd() && _connMap.hasConn(pConn->getConnInfo()))
    {
        _connMap.setConn(pConn->getConnInfo(), nullptr);
        pConn->setConnectCallback(std::bind(&TcpClient::onConnect, this, pConn));
        pConn->setCloseCallback(std::bind(&TcpClient::onClose, this, pConn));
        pConn->doConnect(pConn->getConnInfo());
    }
}
