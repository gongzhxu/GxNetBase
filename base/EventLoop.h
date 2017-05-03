#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <event2/event.h>

#include "CurrentThread.h"
#include "TimerId.h"

class EventLoop
{
public:
    typedef std::function<void()> Functor;
    typedef std::vector<Functor> FunctorList;
    typedef std::map<TimerId, std::unique_ptr<TimerObj> > TimerMap;

    EventLoop(int loopId = 0);
    ~EventLoop();

    void loop();
    void quit();

    struct event_base * get_event() { return base_; }

    inline bool isInLoopThread() const
    {
        return threadId_ == CurrentThread::tid();
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

    TimerId runAfter(const struct timeval & tv, const Functor && cb);
    TimerId runEvery(const struct timeval & tv, const Functor && cb);
    void cancel(TimerId timer);

    void addSignal(evutil_socket_t x, event_callback_fn cb, void * arg);
private:
    void doPendingFunctors();

    void addTimer(TimerId timerId, std::unique_ptr<TimerObj> & timerObj);
    void delTimer(TimerId timerId);

    void wakeup();
    void handleWakeup();

    static void handleWakeup(int fd, short which, void *arg);
private:
    int loopId_;
    int threadId_;

    struct event * wakeupEvent_;
    int wakeupFd_;

    struct event_base * base_;
    bool quit_;

    std::mutex mutex_;
    FunctorList pendingFunctors_;
    size_t sizePendingFunctors_;

    TimerMap    timerMap_;

    std::vector<struct event *> signalEvents_;
    friend TimerObj;
};


#endif
