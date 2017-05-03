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
    TimerObj(EventLoop * loop, TimerId timerId, const struct timeval & tv, const Functor && cb, int type);
public:
    ~TimerObj();

private:
    static TimerId createTimer(EventLoop * loop, const struct timeval & tv, const Functor && cb, int type);
    static void deleteTimer(EventLoop * loop, TimerId timer);

    void onTimer();
    static void startTimer(EventLoop * loop, TimerId timerId, const struct timeval & tv, const Functor & cb, int type);
    static void stopTimer(EventLoop * loop, TimerId timerId);
    static void handleTimer(int fd, short which, void *arg);

private:
    EventLoop *     loop_;
    struct event *  timer_;
    struct timeval   tv_;
    Functor          cb_;
    TimerId          timerId_;
    int              type_;
    friend           EventLoop;
};

#endif
