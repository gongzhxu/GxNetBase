#include "BaseConn.h"

#include <assert.h>

#include "BaseUtil.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "TcpClient.h"

BaseConn::BaseConn(EventLoop * loop):
    _loop(loop),
    _bConnected(false),
    _bClosed(false),
    _bShutdownd(false),
    _bufev(nullptr),
    _tie(nullptr)
{
    LOG_DEBUG("Create Conn:%p", this);
    assert(_loop != nullptr);
}

BaseConn::~BaseConn()
{
    LOG_DEBUG("Delete Conn:%p", this);
}

void BaseConn::sendPdu(const std::shared_ptr<void> & pdu)
{
    _loop->runInLoop(std::bind(&BaseConn::onWrite, shared_from_this(), pdu));
}

bool BaseConn::read(void * data, size_t datlen)
{
    _loop->assertInLoopThread();
    assert(_bufev != nullptr);

    struct evbuffer * inputBuffer = bufferevent_get_input(_bufev);
    if(!inputBuffer)
    {
        LOG_ERROR("read errno=%d, error:%s", errno, strerror(errno));
        close();
        return false;
    }

    int  inputLen = evbuffer_get_length(inputBuffer);
    if(inputLen < static_cast<int>(datlen))
    {
        //input buffer not enough
        return false;
    }

    ASSERT_ABORT(bufferevent_read(_bufev, data, datlen) == datlen);
    return true;
}

bool BaseConn::write(void * data, size_t datlen)
{
    _loop->assertInLoopThread();
    if(!connected())
    {
        return false;
    }

    assert(_bufev != nullptr);
    if(bufferevent_write(_bufev, data, datlen) != 0)
    {
        LOG_ERROR("write errno=%d, error:%s", errno, strerror(errno));
        close();
        return false;
    }

    return true;
}

void BaseConn::close()
{
    //queue in loop is right, or ahaha
    _loop->queueInLoop(std::bind(&BaseConn::closeInLoop, shared_from_this()));
}

void BaseConn::shutdown()
{
    //queue in loop is right, or ahaha
    _bShutdownd = true;
    _loop->queueInLoop(std::bind(&BaseConn::closeInLoop, shared_from_this()));
}

void BaseConn::doAccept(const ConnInfo & ci)
{
    _bClosed = false;
    _connInfo = ci;
    _tie = shared_from_this();
    _loop->runInLoop(std::bind(&BaseConn::BuildAccept, shared_from_this()));
}

void BaseConn::doConnect(const ConnInfo & ci)
{
    _bClosed = false;
    _connInfo = ci;
    _tie = shared_from_this();
    _loop->runInLoop(std::bind(&BaseConn::BuildConnect, shared_from_this()));
}

void BaseConn::onEvent(short what)
{
    if(what & BEV_EVENT_CONNECTED)
    {
        LOG_DEBUG("connect:%d, %d, %s", what, errno, strerror(errno));
        connectInLoop();

    }
    else
    {
        LOG_DEBUG("error:%d, %d, %s", what, errno, strerror(errno));
        closeInLoop();
    }
}

void BaseConn::connectInLoop()
{
    assert(_loop->isInLoopThread());
    _bConnected = true;
    if(_connect_cb)
    {
        _connect_cb(shared_from_this());
    }

    const AddrInfo & addrInfo = _connInfo.getCurrAddrInfo();
    LOG_DEBUG("new conn this=%p, sa_family=%d, ip=%s, port=%d , fd=%d", this, addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port(), _connInfo.fd());

    onConnect();
}

void BaseConn::closeInLoop()
{
    assert(_loop->isInLoopThread());
    if(closed())
    {
        return;
    }

    const AddrInfo & addrInfo = _connInfo.getCurrAddrInfo();
    LOG_DEBUG("del conn this=%p, sa_family=%d, ip=%s, port=%d , fd=%d", this, addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port(), _connInfo.fd());

    onClose();

    if(_close_cb)
    {
        _close_cb(shared_from_this());
    }

    if(_bufev)
    {
        bufferevent_free(_bufev);
        _bufev = nullptr;
    }

    //you can use weakptr,but it too complicate, so you have to free manual
    setConnectCallback(ConnCallback());
    setCloseCallback(ConnCallback());

    _bClosed = true;
    _bConnected = false;
    _tie.reset();
}

void BaseConn::BuildAccept()
{
    assert(_loop->isInLoopThread());
    assert(_bufev == nullptr);

    do
    {
        _bufev = bufferevent_socket_new(_loop->get_event(), _connInfo.fd(), BEV_OPT_CLOSE_ON_FREE);
        if(_bufev == nullptr)
        {
            LOG_ERROR("memory error on  bufferevent_socket_new");
            break;
        }

        bufferevent_setcb(_bufev, read_cb, nullptr, event_cb, this);
        int ret = 0;
        ret = bufferevent_enable(_bufev, EV_READ|EV_WRITE|EV_PERSIST|EV_ET);
        if(ret != 0)
        {
            LOG_ERROR("bufferevent_enable error:%d", ret);
            break;
        }

        connectInLoop();
        return;
    }while(0);

    closeInLoop();
    return;
}

void BaseConn::BuildConnect()
{
    assert(_loop->isInLoopThread());
    assert(_bufev == nullptr);
    do
    {
        _bufev = bufferevent_socket_new(_loop->get_event(), -1, BEV_OPT_CLOSE_ON_FREE);
        if(_bufev == nullptr)
        {
            LOG_ERROR("memory error on  bufferevent_socket_new");
            break;
        }

        bufferevent_setcb(_bufev, read_cb, nullptr, event_cb, this);

        int ret = 0;
        ret = bufferevent_enable(_bufev, EV_READ|EV_WRITE|EV_PERSIST|EV_ET);
        if(ret != 0)
        {
            LOG_ERROR("bufferevent_enable error:%d", ret);
            break;
        }
        //FIXME: please use async resolve the hostname
        const AddrInfo & addrInfo = _connInfo.getNextAddrInfo();
        LOG_INFO("begin connect sa_family=%d, ip=%s, port=%d", addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port());
        ret = bufferevent_socket_connect_hostname(_bufev, nullptr, addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port());
        if(ret != 0)
        {
            LOG_ERROR("bufferevent_socket_connect_hostname error:%d", ret);
            break;
        }

        _connInfo.setFd(bufferevent_getfd(_bufev));
        LOG_DEBUG("connect the server...");
        return;

    }while(0);

    closeInLoop();
    return;
}

void BaseConn::read_cb(struct bufferevent * bev, void * ctx)
{
    NOTUSED_ARG(bev);
    static_cast<BaseConn *>(ctx)->onRead();
}

void BaseConn::event_cb(struct bufferevent * bev, short what, void * ctx)
{
    NOTUSED_ARG(bev);
    static_cast<BaseConn *>(ctx)->onEvent(what);
}

