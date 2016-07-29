#include "TimerId.h"

#include "BaseUtil.h"
#include "EventLoop.h"

TimerId::TimerId(EventLoop * loop, const struct timeval & tv, const Functor & cb, int type):
    _loop(loop),
    _timer(nullptr),
    _tv(tv),
    _cb(cb),
    _type(type)
{

}

TimerId::~TimerId()
{
    _loop->assertInLoopThread();
    _loop->delTimer(this);
    evtimer_del(_timer);
}

TimerId * TimerId::createTimer(EventLoop * loop, const struct timeval & tv, const Functor & cb, int type)
{
    TimerId * pTimer = new TimerId(loop, tv, cb, type);
    pTimer->_loop->runInLoop(std::bind(&TimerId::startTimer, pTimer));
    return pTimer;
}

void TimerId::deleteTimer(TimerId * pTimer)
{
    pTimer->_loop->runInLoop(std::bind(&TimerId::stopTimer, pTimer));
}

void TimerId::startTimer()
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

    _loop->addTimer(this);

    int ret = 0;
    ret = evtimer_add(_timer, &_tv);
    ASSERT_ABORT(ret == 0);
}

void TimerId::onTimer()
{
    _cb();
    if(_type == TIMER_ONCE)
    {
        stopTimer();
    }
}

void TimerId::stopTimer()
{
    delete this;
}

void TimerId::handleTimer(int, short, void * arg)
{
    static_cast<TimerId *>(arg)->onTimer();
}

