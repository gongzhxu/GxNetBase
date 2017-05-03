#ifndef _EVENT_LOOP_THREAD_
#define _EVENT_LOOP_THREAD_

#include <thread>
#include <mutex>
#include <condition_variable>

class EventLoop;

class EventLoopThread
{
public:
    EventLoopThread(int loopId);
    ~EventLoopThread();
public:
    EventLoop * startLoop();

private:
    void threadFunc();

private:
    int         loopId_;
    EventLoop * loop_;

    std::thread thread_;
    std::mutex  mutex_;
    std::condition_variable cond_;
};

#endif
