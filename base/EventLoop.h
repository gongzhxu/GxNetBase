#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <vector>
#include <list>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <event2/event.h>

#include "CurrentThread.h"

class TimerId;

class EventLoop
{
public:
    typedef std::function<void()> Functor;
    typedef std::map<TimerId *, Functor> TimerMap;

    EventLoop(int loopId = 0);
    ~EventLoop();

    void loop();
    void quit();

    struct event_base * get_event() { return _base; }

    inline bool isInLoopThread() const
    {
        return _threadId == CurrentThread::tid();
    }

    inline void assertInLoopThread()
    {
        if(!isInLoopThread())
        {
            abort();
        }
    }

    void runInLoop(const Functor && cb);
    void queueInLoop(const Functor && cb);

    void runAfter(const struct timeval & tv, const Functor & cb);
    TimerId * runEvery(const struct timeval & tv, const Functor & cb);
    void runEveryStop(TimerId * timer);

    void addSignal(evutil_socket_t x, event_callback_fn cb, void * arg);
private:
    void doPendingFunctors();

    void wakeup();
    void handleWakeup();

    void addTimer(TimerId * timer);
    void delTimer(TimerId * timer);

    static void handleWakeup(int fd, short which, void *arg);
private:
    int _loopId;
    int _threadId;

    struct event * _wakeupEvent;
    int _wakeupFd;

    struct event_base * _base;
    bool _quit;

    std::mutex _mutex;
    std::list<Functor> _pendingFunctors;
    bool _callingPendingFunctors;
    size_t _sizePendingFunctors;

    TimerMap    _timerMap;
    friend TimerId;

    std::vector<struct event *> _signalEvents;
};


#endif
