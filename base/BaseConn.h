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

class BaseConn:public std::enable_shared_from_this<BaseConn>
{
public:
    BaseConn(EventLoop * loop);
    virtual ~BaseConn();

public:
    void sendPdu(const std::shared_ptr<void> & pdu);

    bool read(void * data, size_t datlen);
    bool write(void * data, size_t datlen);

    void close();
    void shutdown();

    void doAccept(const ConnInfo & ci);
    void doConnect(const ConnInfo & ci);

    inline EventLoop * getLoop() const { return _loop; }
    inline bool connected() const { return _bConnected; }
    inline bool closed() const { return _bClosed; }
    inline bool shutdownd() const { return _bShutdownd; }
    inline const ConnInfo & getConnInfo() const { return _connInfo; }

    void setConnectCallback(const ConnCallback & cb) { _connect_cb = cb; }
    void setCloseCallback(const ConnCallback & cb) { _close_cb = cb; }
    void setMessageCallback(const ConnCallback & cb) { _message_cb = cb; }
protected:
    virtual void onConnect() {}
    virtual void onClose() {}
    virtual void onRead() {};
    virtual void onWrite(const std::shared_ptr<void> &) {}

private:
    void BuildAccept();
    void BuildConnect();

    void connectInLoop();
    void closeInLoop();
    void onEvent(short what);

    static void read_cb(struct bufferevent * bev, void * ctx);
    static void event_cb(struct bufferevent * bev, short what, void * ctx);
private:
    EventLoop * _loop; // the event loop
    bool _bConnected; // the connect flag
    bool _bClosed; // the close flag
    bool _bShutdownd; // the shutdown flag
    ConnInfo _connInfo; // the connection infomation

    struct bufferevent * _bufev; // the libevent buffer event

    ConnCallback _connect_cb; // register the connet callback
    ConnCallback _close_cb; // register the close callback
    ConnCallback _message_cb;

    //tie 'this', so can't free object manual
    std::shared_ptr<void> _tie;
};

#endif
