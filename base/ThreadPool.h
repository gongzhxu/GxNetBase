#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <assert.h>

class ThreadPool
{
public:
    ThreadPool(size_t threads = 1);
    ~ThreadPool();

    void schedule(std::function<void()> && task)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        assert(!_stop);
        _tasks.emplace(std::move(task));
        _condition.notify_one();
    }

    size_t pending()
    {
        return _tasks.size();
    }
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > _workers;
    // the task queue
    std::queue< std::function<void()> > _tasks;

    // synchronization
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads):_stop(false)
{
    for(size_t i = 0; i < threads; ++i)
        _workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        _condition.wait(lock, [this]{return _stop || !_tasks.empty();});
                        if(_stop &&_tasks.empty())
                        {
                            return;
                        }

                        task = std::move(_tasks.front());
                        _tasks.pop();
                    }

                    task();
                }
            }
        );
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _stop = true;
        _condition.notify_all();
    }

    for(std::thread &worker: _workers)
        worker.join();
}

#endif
