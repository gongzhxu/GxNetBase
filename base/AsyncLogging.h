#ifndef _ASYNC_LOGGING_H_
#define _ASYNC_LOGGING_H_

#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "Logger.h"

class AsyncLogging
{
public:
    typedef std::list<LoggerPtr> LoggerList;

    AsyncLogging(const char * fileName);
    ~AsyncLogging();
public:
    void append(LoggerPtr && logger);
    Logger::LogLevel getLogLevel() const { return _level; }
private:
    void threadFunc();

private:
    std::string _logFolder; // log folder
    std::string _baseName; // log prefix name
    Logger::LogLevel _level; // log level
    size_t _rollSize; // roll size(bytes)
    int _flushInterval; // flush interval
    int _autoRm; // auto remove time(days)
    bool    _print; // print or not
    bool _running; // just a flag indicate the thread is running

    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;

    LoggerList _loggers; // logger list
};

#endif // _ASYNC_LOGGING_H
