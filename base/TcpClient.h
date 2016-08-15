#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <stdint.h>
#include <vector>
#include <memory>

#include "ConnInfo.h"
#include "ConnList.h"
#include "ConnMap.h"

class TcpClient;
class EventLoop;
class BaseConn;

typedef std::shared_ptr<BaseConn> BaseConnPtr;
typedef std::vector<ConnInfo> ConnInfos;
typedef std::vector<BaseConnPtr> ConnList_t;
typedef std::function<void (TcpClient *, const ConnInfo &)> ConnectCallback;

class TcpClient
{
public:
    TcpClient(EventLoop * loop, const ConnectCallback & cb);
    ~TcpClient();

public:
    void addClient(const ConnInfo & ci);
    void delClient(const ConnInfo & ci);

    BaseConnPtr getConn(const ConnInfo & ci);
    BaseConnPtr getNextConn();

    size_t size() { return _connList.size(); }
private:
    void addClientInLoop(const ConnInfo & ci);
    void delClientInLoop(const ConnInfo & ci);
    void onConnect(const BaseConnPtr & pConn);
    void onClose(const BaseConnPtr & pConn);
    void onRetry(const BaseConnPtr & pConn);

private:
    EventLoop * _loop;
    ConnList _connList;
    ConnMap<ConnInfo> _connMap;

    ConnectCallback _connect_cb;
    friend BaseConn;
};

#endif
