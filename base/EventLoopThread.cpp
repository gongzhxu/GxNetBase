#include "EventLoopThread.h"

#include <assert.h>
#include "EventLoop.h"

EventLoopThread::EventLoopThread(int loopId):
    _loopId(loopId),
    _loop(nullptr)
{

}

EventLoopThread::~EventLoopThread()
{
    if(_loop)
    {
        _loop->quit();
    }

    if(_thread.joinable())
        _thread.join();
}

EventLoop * EventLoopThread::startLoop()
{
    assert(_loop == nullptr);

    _thread = std::thread(std::bind(&EventLoopThread::threadFunc, this));

    {
        std::unique_lock<std::mutex> lock(_mutex);
        while(_loop == nullptr)
        {
            _cond.wait(lock);
        }
    }

    return _loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop(_loopId);

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _loop = &loop;
        _cond.notify_one();
    }

    loop.loop();
    _loop = nullptr;
}
