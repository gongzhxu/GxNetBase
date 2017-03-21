#ifndef _TIMER_ID_H_
#define _TIMER_ID_H_

#include <functional>
#include <event2/event.h>

class EventLoop;

enum
{
    TIMER_ONCE = 0,
    TIMER_PERSIST
};

typedef int64_t TimerId;

class TimerObj
{
private:
    typedef std::function<void()> Functor;

    TimerObj(EventLoop * loop, const struct timeval & tv, const Functor && cb, int type);
    ~TimerObj();
public:
    static TimerId createTimer(EventLoop * loop, const struct timeval & tv, const Functor && cb, int type);
    static void deleteTimer(EventLoop * loop, TimerId timer);
private:
    void startTimer();
    void onTimer();

    static void stopTimer(EventLoop * loop, TimerId TimerId);
    static void handleTimer(int fd, short which, void *arg);

public:
    EventLoop *     _loop;
    struct event *  _timer;
    struct timeval   _tv;
    Functor          _cb;
    TimerId          _timerId;
    int              _type;
};

#endif
