#include "TcpClient.h"

#include <assert.h>

#include "BaseUtil.h"
#include "SocketOps.h"

TcpClient::TcpClient(EventLoop * loop):
    loop_(loop)
{
    assert(loop_ != nullptr);
}


TcpClient::~TcpClient()
{

}

void TcpClient::delClient(ConnInfo & ci)
{
    loop_->runInLoop(std::bind(&TcpClient::delClientInLoop, this, ci));
}

BaseConnPtr TcpClient::getConn(ConnInfo & ci)
{
    return connMap_.getConn(ci);
}

BaseConnPtr TcpClient::getNextConn(int type)
{
    return connList_[type].getNextConn();
}

void TcpClient::delClientInLoop(ConnInfo & ci)
{
     BaseConnPtr pConn = connMap_.getConn(ci);
     if(pConn)
     {
         connList_[ci.type()].delConn(pConn);
         pConn->shutdown();
     }

     connMap_.delConn(ci);
}

void TcpClient::onConnect(const BaseConnPtr & pConn)
{
    const ConnInfo & ci = pConn->getConnInfo();
    if(connMap_.hasConn(ci))
    {
        connList_[ci.type()].addConn(pConn);
        BaseConnPtr pOldConn = connMap_.getConn(ci);
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
            connMap_.setConn(ci, pConn);
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

    connList_[ci.type()].delConn(pConn);

    if(pConn->shutdownd())
    {
        connMap_.delConn(ci);
        return;
    }

    struct timeval tv= {ci.retry(), 0};
    loop_->runAfter(tv, std::bind(&TcpClient::onRetry, this, pConn));
}

void TcpClient::onRetry(const BaseConnPtr & pConn)
{
    if(!pConn->shutdownd() && connMap_.hasConn(pConn->getConnInfo()))
    {
        connMap_.setConn(pConn->getConnInfo(), nullptr);
        pConn->setConnectCallback(std::bind(&TcpClient::onConnect, this, pConn));
        pConn->setCloseCallback(std::bind(&TcpClient::onClose, this, pConn));
        pConn->doConnect(pConn->getConnInfo());
    }
}
