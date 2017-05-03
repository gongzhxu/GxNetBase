#ifndef _EVENT_LOOP_THREAD_POOL_
#define _EVENT_LOOP_THREAD_POOL_

#include <vector>
#include <memory>

class BaseConn;
class EventLoop;
class EventLoopThread;

typedef std::shared_ptr<EventLoopThread> EventLoopThreadPtr;
#define MakeEventLoopThreadPtr std::make_shared<EventLoopThread>

class EventLoopThreadPool
{
public:
    EventLoopThreadPool(EventLoop * baseLoop);
    ~EventLoopThreadPool();
public:
    void start(int numThreads);
    void quit();

    EventLoop * getNextLoop();
    EventLoop * getModLoop(int sessionId);
private:
    EventLoop * baseLoop_;
    size_t next_;

    std::vector<EventLoop *> loops_;
    std::vector<EventLoopThreadPtr> threads_;

};

#endif
