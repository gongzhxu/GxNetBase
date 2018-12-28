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
        elt->startLoop();

        threads_.emplace_back(elt);
    }
}

void EventLoopThreadPool::quit()
{
    baseLoop_->assertInLoopThread();
    for(size_t i = 0; i < threads_.size(); ++i)
    {
        threads_[i]->getLoop()->quit();
    }
}

EventLoop * EventLoopThreadPool::getNextLoop()
{
    //baseLoop_->assertInLoopThread();
    EventLoop * loop = baseLoop_;
    if(!threads_.empty())
    {
        if(next_ >= threads_.size())
        {
            next_ = 0;
        }

        loop = threads_[next_++]->getLoop();
    }

    assert(loop != nullptr);
    return loop;
}

EventLoop * EventLoopThreadPool::getModLoop(int sessionId)
{
    //baseLoop_->assertInLoopThread();
    EventLoop * loop = baseLoop_;
    if(!threads_.empty())
    {
        sessionId %= threads_.size();

        loop = threads_[sessionId]->getLoop();
    }

    return loop;
}
