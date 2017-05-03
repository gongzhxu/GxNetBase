#ifndef _ASYNC_LOGGING_H_
#define _ASYNC_LOGGING_H_

#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "Logger.h"
class LogFile;

class AsyncLogging
{
public:
    typedef std::list<LoggerPtr> LoggerList;

    AsyncLogging(const char * fileName);
    ~AsyncLogging();
public:
    void loadConfig(const char * fileName);
    void append(LoggerPtr && logger);
    int getLogLevel() const { return level_; }

private:
    void threadFunc();

private:
    int              flushInterval_;
    int              level_; // log level
    bool            print_; // print or not
    bool            running_; // just a flag indicate the thread is running

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;

    LoggerList                 loggers_; // logger list
    std::unique_ptr<LogFile> output_;
};

#endif // _ASYNC_LOGGING_H
