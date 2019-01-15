#include "EventLoop.h"

#include <assert.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <event2/event.h>

#include "BaseUtil.h"
#include "BaseConn.h"
#include "WeakCallback.h"

EventLoop::EventLoop(int loopId):
    loopId_(loopId),
    threadId_(CurrentThread::tid()),
    wakeupEvent_(nullptr),
    wakeupFd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
    base_(nullptr),
    quit_(false),
    sizePendingFunctors_(0)
{
    ASSERT_ABORT(wakeupFd_ > 0);

    base_ = event_base_new();
    ASSERT_ABORT(base_);

    //'this' contain the '_base' so this is safe
    wakeupEvent_= event_new(base_, wakeupFd_, EV_READ| EV_PERSIST, handleWakeup, this);
    ASSERT_ABORT(wakeupEvent_);
    ASSERT_ABORT(event_add(wakeupEvent_, nullptr) == 0);
}

EventLoop::~EventLoop()
{
    event_base_free(base_);
    event_free(wakeupEvent_);
    ::close(wakeupFd_);
    wakeupFd_ = -1;

    for(size_t i = 0; i < signalEvents_.size(); ++i)
    {
        evsignal_del(signalEvents_[i]);
    }
}

void EventLoop::loop()
{
    assert(base_ != nullptr);

    while(!quit_)
    {
        event_base_loop(base_, EVLOOP_ONCE);
        doPendingFunctors();
    }

    LOG_INFO("loop quited %p, pendingFunctors_=%d", this, sizePendingFunctors_);

    quit_ = false;

    std::unique_lock<std::mutex> lock(mutex_);
    sizePendingFunctors_ = 0;
    pendingFunctors_.clear();
}

//not thread safe, please close eventloop in the loop thread
void EventLoop::quit()
{
    LOG_DEBUG("event loop quiting %p...", this);
    quit_ = true;
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
        std::unique_lock<std::mutex> lock(mutex_);
        sizePendingFunctors = sizePendingFunctors_++;
        pendingFunctors_.emplace_back(std::move(cb));
    }

    if(sizePendingFunctors == 0)
    {
        wakeup();
    }
}

TimerId EventLoop::runAfter(const struct timeval & tv, const Functor && cb)
{
    return TimerObj::createTimer(this, tv, std::move(cb), TIMER_ONCE);
}

TimerId  EventLoop::runEvery(const struct timeval & tv, const Functor && cb)
{
    return TimerObj::createTimer(this, tv, std::move(cb), TIMER_PERSIST);
}

void EventLoop::cancel(TimerId timer)
{
    TimerObj::deleteTimer(this, timer);
}

void EventLoop::addSignal(int x, signal_callback_fn cb, void * arg)
{
    struct event * se = evsignal_new(base_, x, cb, arg);
    ASSERT_ABORT(se);
    evsignal_add(se, nullptr);
    signalEvents_.emplace_back(se);
}

void EventLoop::doPendingFunctors()
{
    FunctorList functors;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        sizePendingFunctors_ = 0;
        functors.swap(pendingFunctors_);
    }

    for(FunctorList::iterator it = functors.begin(); it != functors.end(); ++it)
    {
        (*it)();
    }
}

void EventLoop::addTimer(TimerId timerId, std::unique_ptr<TimerObj> & timerObj)
{
    timerMap_.insert(std::make_pair(timerId, std::move(timerObj)));
}

void EventLoop::delTimer(TimerId timerId)
{
    timerMap_.erase(timerId);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_FATAL("wakeup error:%d, %s", n, strerror(errno));
    }
}

void EventLoop::handleWakeup()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
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

