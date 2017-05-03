#include "TcpServer.h"

#include <assert.h>

#include "BaseUtil.h"

TcpServer::TcpServer(EventLoop * loop):
    loop_(loop),
    listener_(nullptr)
{
    assert(loop_ != nullptr);
}

TcpServer::~TcpServer()
{
}

void TcpServer::delServer(ConnInfo & ci)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connList_.erase(ci);
    }
    loop_->runInLoop(std::bind(&TcpServer::delServerInLoop, this, ci));
}

void TcpServer::delServerInLoop(ConnInfo & ci)
{
    auto it = listeners_.find(ci);
    if(it != listeners_.end() && it->second)
    {
        struct evconnlistener * listener = it->second;
        if(listener)
        {
            evconnlistener_free(listener_);
            listeners_.erase(it);
        }
    }
}

void TcpServer::getConnInfo(std::vector<ConnInfo> & connList)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for(auto it = connList_.begin(); it != connList_.end(); ++it)
    {
        connList.push_back(*it);
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

void TcpServer::onMessage(const BaseConnPtr &)
{

}
