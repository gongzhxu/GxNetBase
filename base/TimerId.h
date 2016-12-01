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

class TimerId
{
private:
    typedef std::function<void()> Functor;

    TimerId(EventLoop * loop, const struct timeval & tv, const Functor & cb, int type);
    ~TimerId();
public:
    static TimerId * createTimer(EventLoop * loop, const struct timeval & tv, const Functor & cb, int type);
    static void deleteTimer(TimerId * timer);
private:
    void startTimer();
    void onTimer();
    void stopTimer();

    static void handleTimer(int fd, short which, void *arg);

public:
    EventLoop * _loop;
    struct event * _timer;
    struct timeval  _tv;
    Functor _cb;

    int _type;
};

#endif
