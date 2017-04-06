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
    int getLogLevel() const { return _level; }

private:
    void threadFunc();

private:
    int              _flushInterval;
    int              _level; // log level
    bool            _print; // print or not
    bool            _running; // just a flag indicate the thread is running

    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;

    LoggerList                 _loggers; // logger list
    std::unique_ptr<LogFile> _output;
};

#endif // _ASYNC_LOGGING_H
