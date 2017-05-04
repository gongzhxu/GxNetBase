#include "TimerId.h"

#include <atomic>

#include "BaseUtil.h"
#include "EventLoop.h"

TimerObj::TimerObj(EventLoop * loop, TimerId timerId, const struct timeval & tv, const Functor && cb, int type):
    loop_(loop),
    timer_(nullptr),
    tv_(tv),
    cb_(std::move(cb)),
    timerId_(timerId),
    type_(type)
{
    loop_->assertInLoopThread();
    if(type_ == TIMER_ONCE)
    {
        timer_ = evtimer_new(loop_->get_event(), handleTimer, this);
    }
    else
    {
        timer_ = event_new(loop_->get_event(), -1, EV_PERSIST, handleTimer, this);
    }
    ASSERT_ABORT(timer_);
    evtimer_add(timer_, &tv_);
}

TimerObj::~TimerObj()
{
    loop_->assertInLoopThread();
    evtimer_del(timer_);
    event_free(timer_);
}

TimerId TimerObj::createTimer(EventLoop * loop, const struct timeval & tv, const Functor && cb, int type)
{
    static std::atomic<TimerId> g_timerId(0);
    TimerId timerId = ++g_timerId;
    loop->runInLoop(std::bind(&TimerObj::startTimer, loop, timerId, tv, cb, type));
    return timerId;
}

void TimerObj::deleteTimer(EventLoop * loop, TimerId timerId)
{
    loop->runInLoop(std::bind(&TimerObj::stopTimer, loop, timerId));
}

void TimerObj::startTimer(EventLoop * loop, TimerId timerId, const struct timeval & tv, const Functor & cb, int type)
{
    std::unique_ptr<TimerObj> timerObj(new TimerObj(loop, timerId, tv, std::move(cb), type));
    loop->addTimer(timerId, timerObj);
}

void TimerObj::stopTimer(EventLoop * loop, TimerId TimerId)
{
    loop->delTimer(TimerId);
}

void TimerObj::onTimer()
{
    cb_();
    if(type_ == TIMER_ONCE)
    {
        stopTimer(loop_, timerId_);
    }
}

void TimerObj::handleTimer(int, short, void * arg)
{
    static_cast<TimerObj *>(arg)->onTimer();
}

