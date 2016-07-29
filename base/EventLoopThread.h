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
    int         _loopId;
    EventLoop * _loop;

    std::thread _thread;
    std::mutex  _mutex;
    std::condition_variable _cond;
};

#endif
