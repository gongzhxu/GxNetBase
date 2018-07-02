#ifndef _BASE_CONN_H_
#define _BASE_CONN_H_

#include <map>
#include <memory>

#include "ConnInfo.h"

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
    bool write(void * data1, size_t datlen1, void * data2, size_t datlen2);

    void close();
    void shutdown();

    void doAccept(const ConnInfo & ci);
    void doConnect(const ConnInfo & ci);

    inline EventLoop * getLoop() const { return loop_; }
    inline bool connected() const { return bConnected_; }
    inline bool closed() const { return bClosed_; }
    inline bool shutdownd() const { return bShutdownd_; }
    inline const ConnInfo & getConnInfo() const { return connInfo_; }

    void setConnectCallback(const ConnCallback & cb) { connect_cb_ = cb; }
    void setCloseCallback(const ConnCallback & cb) { close_cb_ = cb; }
    void setMessageCallback(const ConnCallback & cb) { message_cb_ = cb; }
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
    EventLoop * loop_; // the event loop
    bool bConnected_; // the connect flag
    bool bClosed_; // the close flag
    bool bShutdownd_; // the shutdown flag
    ConnInfo connInfo_; // the connection infomation

    struct bufferevent * bufev_; // the libevent buffer event

    ConnCallback connect_cb_; // register the connet callback
    ConnCallback close_cb_; // register the close callback
    ConnCallback message_cb_;

    //tie 'this', so can't free object manual
    std::shared_ptr<void> tie_;
};

#endif
