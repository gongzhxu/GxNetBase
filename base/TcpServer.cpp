#include "TcpServer.h"

#include <assert.h>

#include "BaseUtil.h"

TcpServer::TcpServer(EventLoop * loop):
    _loop(loop),
    _listener(nullptr)
{
    assert(_loop != nullptr);
}

TcpServer::~TcpServer()
{
}

void TcpServer::delServer(ConnInfo & ci)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _connList.erase(ci);
    }
    _loop->runInLoop(std::bind(&TcpServer::delServerInLoop, this, ci));
}

void TcpServer::delServerInLoop(ConnInfo & ci)
{
    auto it = _listeners.find(ci);
    if(it != _listeners.end() && it->second)
    {
        struct evconnlistener * listener = it->second;
        if(listener)
        {
            evconnlistener_free(_listener);
            _listeners.erase(it);
        }
    }
}

void TcpServer::getConnInfo(std::vector<ConnInfo> & connList)
{
    std::unique_lock<std::mutex> lock(_mutex);
    for(auto it = _connList.begin(); it != _connList.end(); ++it)
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
