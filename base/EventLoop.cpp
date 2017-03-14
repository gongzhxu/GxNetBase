#include "EventLoop.h"

#include <assert.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include "BaseUtil.h"
#include "BaseConn.h"
#include "TimerId.h"
#include "WeakCallback.h"

EventLoop::EventLoop(int loopId):
    _loopId(loopId),
    _threadId(CurrentThread::tid()),
    _wakeupEvent(nullptr),
    _wakeupFd(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
    _base(nullptr),
    _quit(false),
    _sizePendingFunctors(0)
{
    ASSERT_ABORT(_wakeupFd > 0);

    _base = event_base_new();
    ASSERT_ABORT(_base);

    //'this' contain the '_base' so this is safe
    _wakeupEvent= event_new(_base, _wakeupFd, EV_READ| EV_PERSIST, handleWakeup, this);
    ASSERT_ABORT(_wakeupEvent);
    ASSERT_ABORT(event_add(_wakeupEvent, nullptr) == 0);
}

EventLoop::~EventLoop()
{
    event_base_free(_base);
    event_free(_wakeupEvent);
    ::close(_wakeupFd);
    _wakeupFd = -1;

    for(size_t i = 0; i < _signalEvents.size(); ++i)
    {
        evsignal_del(_signalEvents[i]);
    }
}

void EventLoop::loop()
{
    assert(_base != nullptr);

    while(!_quit)
    {
        event_base_loop(_base, EVLOOP_ONCE);
        doPendingFunctors();
    }
    LOG_INFO("loop quited %p, _pendingFunctors=%d", this, _sizePendingFunctors);
}

//not thread safe, please close eventloop in the loop thread
void EventLoop::quit()
{
    LOG_DEBUG("event loop quiting %p...", this);
    _quit = true;
    wakeup();
}

//if in loop thread this call immediately, else queue in loop
void EventLoop::runInLoop(const Functor && cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

//called when the loop event end
void EventLoop::queueInLoop(const Functor && cb)
{
    size_t sizePendingFunctors = 0;

    {
        std::unique_lock<std::mutex> lock(_mutex);
        sizePendingFunctors = _sizePendingFunctors++;
        _pendingFunctors.push_back(std::move(cb));
    }

    if(sizePendingFunctors == 0)
    {
        wakeup();
    }
}

void EventLoop::runAfter(const struct timeval & tv, const Functor & cb)
{
    TimerId::createTimer(this, tv, cb, TIMER_ONCE);
}

TimerId * EventLoop::runEvery(const struct timeval & tv, const Functor & cb)
{
    return TimerId::createTimer(this, tv, cb, TIMER_PERSIST);;
}

void EventLoop::runEveryStop(TimerId * timer)
{
    return TimerId::deleteTimer(timer);
}

void EventLoop::addSignal(evutil_socket_t x, event_callback_fn cb, void * arg)
{
    struct event * se = evsignal_new(_base, x, cb, arg);
    ASSERT_ABORT(se);
    evsignal_add(se, nullptr);
    _signalEvents.push_back(se);
}

void EventLoop::doPendingFunctors()
{
    FunctorList functors;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _sizePendingFunctors = 0;
        functors.swap(_pendingFunctors);
    }

    for(FunctorList::iterator it = functors.begin(); it != functors.end(); ++it)
    {
        (*it)();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(_wakeupFd, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_FATAL("wakeup error:%d, %s", n, strerror(errno));
    }
}

void EventLoop::addTimer(TimerId * timer)
{
    assert(_timerMap[timer] == nullptr);
    _timerMap[timer] = timer->_cb;
}


void EventLoop::delTimer(TimerId * timer)
{
    //assert(_timerMap[timer] == timer->_cb);
    _timerMap.erase(timer);
}

void EventLoop::handleWakeup()
{
    uint64_t one = 1;
    ssize_t n = ::read(_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_FATAL("handleRead error:%d, %s", n, strerror(errno));
    }
}

void EventLoop::handleWakeup(int fd, short which, void * arg)
{
    NOTUSED_ARG(fd);
    NOTUSED_ARG(which);
    static_cast<EventLoop *>(arg)->handleWakeup();
}

