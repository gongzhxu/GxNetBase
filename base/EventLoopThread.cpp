#include "EventLoopThread.h"

#include <assert.h>


EventLoopThread::EventLoopThread(int loopId):
    loop_(loopId)
{

}

EventLoopThread::~EventLoopThread()
{
    bool _isInLoopThread = loop_.isInLoopThread();
    loop_.quit();


    if(thread_.joinable())
    {
        if(_isInLoopThread)
        {
            thread_.detach();
        }
        else
        {
            thread_.join();
        }
    }
}

EventLoop * EventLoopThread::startLoop()
{
    if(!thread_.joinable())
    {
        thread_ = std::thread(std::bind(&EventLoopThread::threadFunc, this));
    }

    return &loop_;
}

void EventLoopThread::stopLoop()
{
    assert(!loop_.isInLoopThread());

    loop_.quit();

    if(thread_.joinable())
        thread_.join();
}

EventLoop * EventLoopThread::getLoop()
{
    return &loop_;
}

void EventLoopThread::threadFunc()
{
    loop_.loop();
}
