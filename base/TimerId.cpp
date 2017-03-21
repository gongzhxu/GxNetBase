#include "TimerId.h"

#include <atomic>

#include "BaseUtil.h"
#include "EventLoop.h"

static std::atomic<TimerId> g_timerId(0);

TimerObj::TimerObj(EventLoop * loop, const struct timeval & tv, const Functor && cb, int type):
    _loop(loop),
    _timer(nullptr),
    _tv(tv),
    _cb(std::move(cb)),
    _timerId(++g_timerId),
    _type(type)
{

}

TimerObj::~TimerObj()
{
    _loop->assertInLoopThread();
    _loop->delTimer(_timerId);
    evtimer_del(_timer);
}

TimerId TimerObj::createTimer(EventLoop * loop, const struct timeval & tv, const Functor && cb, int type)
{
    TimerObj * pTimer = new TimerObj(loop, tv, std::move(cb), type);
    loop->runInLoop(std::bind(&TimerObj::startTimer, pTimer));
    return pTimer->_timerId;
}

void TimerObj::deleteTimer(EventLoop * loop, TimerId TimerId)
{
    loop->runInLoop(std::bind(&TimerObj::stopTimer, loop, TimerId));
}

void TimerObj::startTimer()
{
    if(_type == TIMER_ONCE)
    {
        _timer = evtimer_new(_loop->get_event(), handleTimer, this);
    }
    else
    {
        _timer = event_new(_loop->get_event(), -1, EV_PERSIST, handleTimer, this);
    }
    ASSERT_ABORT(_timer);

    _loop->addTimer(_timerId, this);
    int ret = evtimer_add(_timer, &_tv);
    ASSERT_ABORT(ret == 0);
}

void TimerObj::onTimer()
{
    _cb();
    if(_type == TIMER_ONCE)
    {
        stopTimer(_loop, _timerId);
    }
}

void TimerObj::stopTimer(EventLoop * loop, TimerId TimerId)
{
    TimerObj * timerObj = loop->getTimer(TimerId);
    if(timerObj)
    {
        delete timerObj;
    }
}

void TimerObj::handleTimer(int, short, void * arg)
{
    static_cast<TimerObj *>(arg)->onTimer();
}

