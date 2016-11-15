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

    AsyncLogging(const char * szCfgFile);
    ~AsyncLogging();
public:
    void append(LoggerPtr && logger);
    Logger::LogLevel getLogLevel() const { return _level; }
private:
    void threadFunc();

private:
    std::string _basename;
    Logger::LogLevel _level;
    size_t _rollSize;
    int _flushInterval;
    int _autoRm;
    bool    _print;
    bool _running;

    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;

    LoggerList _loggers;
};

#endif // _ASYNC_LOGGING_H
