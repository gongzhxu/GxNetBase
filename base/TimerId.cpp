#include "TimerId.h"

#include <atomic>

#include "BaseUtil.h"
#include "EventLoop.h"

TimerObj::TimerObj(EventLoop * loop, TimerId timerId, const struct timeval & tv, const Functor && cb, int type):
    _loop(loop),
    _timer(nullptr),
    _tv(tv),
    _cb(std::move(cb)),
    _timerId(timerId),
    _type(type)
{
    _loop->assertInLoopThread();
    if(_type == TIMER_ONCE)
    {
        _timer = evtimer_new(_loop->get_event(), handleTimer, this);
    }
    else
    {
        _timer = event_new(_loop->get_event(), -1, EV_PERSIST, handleTimer, this);
    }
    ASSERT_ABORT(_timer);
    evtimer_add(_timer, &_tv);
}

TimerObj::~TimerObj()
{
    _loop->assertInLoopThread();
    evtimer_del(_timer);
    event_free(_timer);
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
    _cb();
    if(_type == TIMER_ONCE)
    {
        stopTimer(_loop, _timerId);
    }
}

void TimerObj::handleTimer(int, short, void * arg)
{
    static_cast<TimerObj *>(arg)->onTimer();
}

