#include "EventLoopThreadPool.h"

#include <assert.h>

#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop * baseLoop):
    baseLoop_(baseLoop),
    next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(int numThreads)
{
    baseLoop_->assertInLoopThread();
    for(int i = 0; i < numThreads; ++i)
    {
        EventLoopThreadPtr elt(MakeEventLoopThreadPtr(i));
        threads_.emplace_back(elt);
        loops_.emplace_back(elt->startLoop());
    }
}

void EventLoopThreadPool::quit()
{
    baseLoop_->assertInLoopThread();
    for(size_t i = 0; i < loops_.size(); ++i)
    {
        loops_[i]->quit();
    }
}

EventLoop * EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    EventLoop * loop = baseLoop_;
    if(!loops_.empty())
    {
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }

        loop = loops_[next_++];
    }

    assert(loop != nullptr);
    return loop;
}

EventLoop * EventLoopThreadPool::getModLoop(int sessionId)
{
    baseLoop_->assertInLoopThread();
    EventLoop * loop = baseLoop_;
    if(!loops_.empty())
    {
        sessionId %= loops_.size();

        loop = loops_[sessionId];
    }

    return loop;
}
