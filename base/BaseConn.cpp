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
    _sockfd(-1),
    _bufev(nullptr),
    _inputBuffer(nullptr),
    _tie(nullptr)
{
    LOG_DEBUG("Create Conn:%p", this);
    assert(_loop != nullptr);
}

BaseConn::~BaseConn()
{
    LOG_DEBUG("Delete Conn:%p", this);
}

int BaseConn::readInput()
{
    _loop->assertInLoopThread();
    assert(_bufev != nullptr && _inputBuffer != nullptr);

    int ret = bufferevent_read_buffer(_bufev, _inputBuffer);
    if(ret != 0)
    {
        LOG_INFO("read error:%s", strerror(errno));
        close();
        return -1;
    }

    return evbuffer_get_length(_inputBuffer);
}

int BaseConn::readbuf(void *data, size_t datlen)
{
    _loop->assertInLoopThread();
    assert(_bufev != nullptr && _inputBuffer != nullptr);

    return evbuffer_remove(_inputBuffer, data, datlen);
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

void BaseConn::doAccept(TcpServer * pServer, evutil_socket_t sockfd)
{
    _sockfd = sockfd;
    _tie = shared_from_this();
    setConnectCallback(std::bind(&TcpServer::onConnect, pServer, shared_from_this()));
    setCloseCallback(std::bind(&TcpServer::onClose, pServer, shared_from_this()));
    _loop->runInLoop(std::bind(&BaseConn::BuildAccept, shared_from_this()));
}

void BaseConn::doConnect(TcpClient * pClient, const ConnInfo & ci)
{
    _connInfo = ci;
    _tie = shared_from_this();
    setConnectCallback(std::bind(&TcpClient::onConnect, pClient, shared_from_this()));
    setCloseCallback(std::bind(&TcpClient::onClose, pClient, shared_from_this()));
    _loop->runInLoop(std::bind(&BaseConn::BuildConnect, shared_from_this()));
}

void BaseConn::onEvent(short what)
{
    if(what & BEV_EVENT_CONNECTED)
    {
        LOG_DEBUG("connect:%s", strerror(errno));
        connectInLoop();

    }
    else  if(what & (BEV_EVENT_READING | BEV_EVENT_WRITING | BEV_EVENT_EOF | BEV_EVENT_ERROR))
    {
        LOG_DEBUG("error:%s", strerror(errno));
        closeInLoop();
    }
    else
    {
        //can't do here
        LOG_ERROR("unknown error:%s", strerror(errno));
        abort();
    }
}

void BaseConn::connectInLoop()
{
    assert(_loop->isInLoopThread());
    _sockfd = bufferevent_getfd(_bufev);
    _bConnected = true;
    assert(_connect_cb);
    _connect_cb(shared_from_this());

    struct sockaddr_in addr = base::getPeerAddr(_sockfd);
    LOG_DEBUG("new conn %s:%d this=%p, fd=%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), this, _sockfd);

    onConnect();
}

void BaseConn::closeInLoop()
{
    assert(_loop->isInLoopThread());
    if(_bClosed)
    {
        return;
    }
    _bClosed = true;

    struct sockaddr_in addr = base::getPeerAddr(_sockfd);
    LOG_DEBUG("del conn %s:%d this=%p, fd=%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), this, _sockfd);

    onClose();

    assert(_close_cb);
    _close_cb(shared_from_this());

    if(_bufev)
    {
        bufferevent_free(_bufev);
        _bufev = nullptr;
    }

    if(_inputBuffer)
    {
        evbuffer_free(_inputBuffer);
        _inputBuffer = nullptr;
    }

    //you can use weakptr,but it too complicate, so you have to free manual
    setConnectCallback(ConnCallback());
    setCloseCallback(ConnCallback());

    _bConnected = false;
    _sockfd = -1;
    _tie.reset();
}

void BaseConn::sendInLoop(const void *data, size_t datlen)
{
    assert(_loop->isInLoopThread());
    if(!connected())
    {
        return;
    }
    assert(_bufev != nullptr && _inputBuffer != nullptr);
    struct evbuffer * buf = bufferevent_get_output(_bufev);
    ASSERT_ABORT(evbuffer_add(buf, data, datlen) == 0);
}

void BaseConn::sendInLoop(const void *data1, size_t datlen1, const void *data2, size_t datlen2)
{
    assert(_loop->isInLoopThread());
    if(!connected())
    {
        return;
    }

    assert(_bufev != nullptr && _inputBuffer != nullptr);
    struct evbuffer * buf = bufferevent_get_output(_bufev);
    ASSERT_ABORT(evbuffer_expand(buf, datlen1+datlen2) == 0);
    ASSERT_ABORT(evbuffer_add(buf, data1, datlen1) == 0);
    ASSERT_ABORT(evbuffer_add(buf, data2, datlen2) == 0);
}

void BaseConn::BuildAccept()
{
    assert(_loop->isInLoopThread());
    assert(_bufev == nullptr && _inputBuffer == nullptr);

    do
    {
        _bufev = bufferevent_socket_new(_loop->get_event(), _sockfd, BEV_OPT_CLOSE_ON_FREE);
        if(_bufev == nullptr)
        {
            LOG_ERROR("memory error on  bufferevent_socket_new");
            break;
        }

        _inputBuffer = evbuffer_new();
        if(_inputBuffer == nullptr)
        {
            LOG_ERROR("memory error on  evbuffer_new");
            break;
        }

        bufferevent_setcb(_bufev, read_cb, write_cb, event_cb, this);
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
    assert(_bufev == nullptr && _inputBuffer == nullptr);
    do
    {
        _bufev = bufferevent_socket_new(_loop->get_event(), -1, BEV_OPT_CLOSE_ON_FREE);
        if(_bufev == nullptr)
        {
            LOG_ERROR("memory error on  bufferevent_socket_new");
            break;
        }

        _inputBuffer = evbuffer_new();
        if(_inputBuffer == nullptr)
        {
            LOG_ERROR("memory error on  evbuffer_new");
            break;
        }

        bufferevent_setcb(_bufev, read_cb, write_cb, event_cb, this);

        int ret = 0;
        ret = bufferevent_enable(_bufev, EV_READ|EV_WRITE|EV_PERSIST|EV_ET);
        if(ret != 0)
        {
            LOG_ERROR("bufferevent_enable error:%d", ret);
            break;
        }
        //please use async resolve the hostname
        ret = bufferevent_socket_connect_hostname(_bufev, nullptr, AF_INET, _connInfo.host().c_str(), _connInfo.port());
        if(ret != 0)
        {
            LOG_ERROR("bufferevent_socket_connect_hostname error:%d", ret);
            break;
        }

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

void BaseConn::write_cb(struct bufferevent * bev, void *ctx)
{
    NOTUSED_ARG(bev);
    static_cast<BaseConn *>(ctx)->onWrite();
}

void BaseConn::event_cb(struct bufferevent * bev, short what, void * ctx)
{
    NOTUSED_ARG(bev);
    static_cast<BaseConn *>(ctx)->onEvent(what);
}

