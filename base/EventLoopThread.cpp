#include "EventLoopThread.h"

#include <assert.h>
#include "EventLoop.h"

EventLoopThread::EventLoopThread(int loopId):
    loopId_(loopId),
    loop_(nullptr)
{

}

EventLoopThread::~EventLoopThread()
{
    bool _isInLoopThread = false;

    if(loop_)
    {
        _isInLoopThread = loop_->isInLoopThread();
        loop_->quit();
    }

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
    assert(loop_ == nullptr);

    thread_ = std::thread(std::bind(&EventLoopThread::threadFunc, this));

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);
        }
    }

    return loop_.get();
}

void EventLoopThread::stopLoop()
{
    assert(loop_ && !loop_->isInLoopThread());

    loop_->quit();

    if(thread_.joinable())
        thread_.join();
}

EventLoop * EventLoopThread::getLoop()
{
    assert(loop_);
    return loop_.get();
}

void EventLoopThread::threadFunc()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_.reset(new EventLoop(loopId_));
        cond_.notify_all();
    }

    loop_->loop();
}
