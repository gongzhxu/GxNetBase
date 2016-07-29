#ifndef _BASE_CONN_H_
#define _BASE_CONN_H_

#include <map>
#include <memory>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "ConnInfo.h"

class TcpServer;
class TcpClient;
class BaseConn;
class EventLoop;

typedef std::shared_ptr<BaseConn> BaseConnPtr;
typedef std::map<uint32_t,  BaseConnPtr> ConnMap_t;
typedef std::function<void (const BaseConnPtr &)> ConnCallback;

#define MakePduBasePtr std::make_shared<PduBase>

class BaseConn:public std::enable_shared_from_this<BaseConn>
{
public:
    BaseConn(EventLoop * loop);
    virtual ~BaseConn();

public:
    int readInput();
    int readbuf(void *data, size_t datlen);
    void close();
    void shutdown();

    void doAccept(TcpServer * pServer, evutil_socket_t sockfd);
    void doConnect(TcpClient * pClient, const ConnInfo & ci);

    inline EventLoop * getLoop() { return _loop; }
    inline bool connected() { return _bConnected; }
    inline bool closed() { return _bClosed; }
    inline bool shutdownd() { return _bShutdownd; }
    inline int getSockfd() { return _sockfd; }
    inline ConnInfo & getConnInfo() { return _connInfo; }

protected:
    virtual void onRead() {};
    virtual void onWrite() {};
    virtual void onEvent(short what);

    virtual void onConnect() {}
    virtual void onClose() {}


    void connectInLoop();
    void closeInLoop();
    void sendInLoop(const void *data, size_t datlen);
    void sendInLoop(const void *data1, size_t datlen1, const void *data2, size_t datlen2);
private:
    void BuildAccept();
    void BuildConnect();

    static void read_cb(struct bufferevent * bev, void * ctx);
    static void write_cb(struct bufferevent * bev, void * ctx);
    static void event_cb(struct bufferevent * bev, short what, void * ctx);

    void setConnectCallback(const ConnCallback & cb) { _connect_cb = cb; }
    void setCloseCallback(const ConnCallback & cb) { _close_cb = cb; }
private:
    EventLoop * _loop;
    bool _bConnected;
    bool _bClosed;
    bool _bShutdownd;
    evutil_socket_t _sockfd;
    ConnInfo _connInfo;

    struct bufferevent * _bufev;
    struct evbuffer * _inputBuffer;

    ConnCallback _connect_cb;
    ConnCallback _close_cb;

    //tie 'this', so can't free manual
    std::shared_ptr<void> _tie;
};

#endif
