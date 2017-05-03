#include "BaseConn.h"

#include <assert.h>

#include "BaseUtil.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "TcpClient.h"

BaseConn::BaseConn(EventLoop * loop):
    loop_(loop),
    bConnected_(false),
    bClosed_(false),
    bShutdownd_(false),
    bufev_(nullptr),
    tie_(nullptr)
{
    LOG_DEBUG("Create Conn:%p", this);
    assert(loop_ != nullptr);
}

BaseConn::~BaseConn()
{
    LOG_DEBUG("Delete Conn:%p", this);
}

void BaseConn::sendPdu(const std::shared_ptr<void> & pdu)
{
    loop_->runInLoop(std::bind(&BaseConn::onWrite, shared_from_this(), pdu));
}

bool BaseConn::read(void * data, size_t datlen)
{
    loop_->assertInLoopThread();
    assert(bufev_ != nullptr);

    struct evbuffer * inputBuffer = bufferevent_get_input(bufev_);
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

    ASSERT_ABORT(bufferevent_read(bufev_, data, datlen) == datlen);
    return true;
}

bool BaseConn::write(void * data, size_t datlen)
{
    loop_->assertInLoopThread();
    if(!connected())
    {
        return false;
    }

    assert(bufev_ != nullptr);
    if(bufferevent_write(bufev_, data, datlen) != 0)
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
    loop_->queueInLoop(std::bind(&BaseConn::closeInLoop, shared_from_this()));
}

void BaseConn::shutdown()
{
    //queue in loop is right, or ahaha
    bShutdownd_ = true;
    loop_->queueInLoop(std::bind(&BaseConn::closeInLoop, shared_from_this()));
}

void BaseConn::doAccept(const ConnInfo & ci)
{
    bClosed_ = false;
    connInfo_ = ci;
    tie_ = shared_from_this();
    loop_->runInLoop(std::bind(&BaseConn::BuildAccept, shared_from_this()));
}

void BaseConn::doConnect(const ConnInfo & ci)
{
    bClosed_ = false;
    connInfo_ = ci;
    tie_ = shared_from_this();
    loop_->runInLoop(std::bind(&BaseConn::BuildConnect, shared_from_this()));
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
    assert(loop_->isInLoopThread());
    bConnected_ = true;
    if(connect_cb_)
    {
        connect_cb_(shared_from_this());
    }

    const AddrInfo & addrInfo = connInfo_.getCurrAddrInfo();
    LOG_DEBUG("new conn this=%p, sa_family=%d, ip=%s, port=%d , fd=%d", this, addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port(), connInfo_.fd());

    onConnect();
}

void BaseConn::closeInLoop()
{
    assert(loop_->isInLoopThread());
    if(closed())
    {
        return;
    }

    const AddrInfo & addrInfo = connInfo_.getCurrAddrInfo();
    LOG_DEBUG("del conn this=%p, sa_family=%d, ip=%s, port=%d , fd=%d", this, addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port(), connInfo_.fd());

    onClose();

    if(close_cb_)
    {
        close_cb_(shared_from_this());
    }

    if(bufev_)
    {
        bufferevent_free(bufev_);
        bufev_ = nullptr;
    }

    //you can use weakptr,but it too complicate, so you have to free manual
    setConnectCallback(ConnCallback());
    setCloseCallback(ConnCallback());

    bClosed_ = true;
    bConnected_ = false;
    tie_.reset();
}

void BaseConn::BuildAccept()
{
    assert(loop_->isInLoopThread());
    assert(bufev_ == nullptr);

    do
    {
        bufev_ = bufferevent_socket_new(loop_->get_event(), connInfo_.fd(), BEV_OPT_CLOSE_ON_FREE);
        if(bufev_ == nullptr)
        {
            LOG_ERROR("memory error on  bufferevent_socket_new");
            break;
        }

        bufferevent_setcb(bufev_, read_cb, nullptr, event_cb, this);
        int ret = 0;
        ret = bufferevent_enable(bufev_, EV_READ|EV_WRITE|EV_PERSIST|EV_ET);
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
    assert(loop_->isInLoopThread());
    assert(bufev_ == nullptr);
    do
    {
        bufev_ = bufferevent_socket_new(loop_->get_event(), -1, BEV_OPT_CLOSE_ON_FREE);
        if(bufev_ == nullptr)
        {
            LOG_ERROR("memory error on  bufferevent_socket_new");
            break;
        }

        bufferevent_setcb(bufev_, read_cb, nullptr, event_cb, this);

        int ret = 0;
        ret = bufferevent_enable(bufev_, EV_READ|EV_WRITE|EV_PERSIST|EV_ET);
        if(ret != 0)
        {
            LOG_ERROR("bufferevent_enable error:%d", ret);
            break;
        }
        //FIXME: please use async resolve the hostname
        const AddrInfo & addrInfo = connInfo_.getNextAddrInfo();
        LOG_INFO("begin connect sa_family=%d, ip=%s, port=%d", addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port());
        ret = bufferevent_socket_connect_hostname(bufev_, nullptr, addrInfo.sa_family(), addrInfo.ip().c_str(), addrInfo.port());
        if(ret != 0)
        {
            LOG_ERROR("bufferevent_socket_connecthostname_ error:%d", ret);
            break;
        }

        connInfo_.setFd(bufferevent_getfd(bufev_));
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

