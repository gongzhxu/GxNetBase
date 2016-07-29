#ifndef _ASYNC_LOGGING_H_
#define _ASYNC_LOGGING_H_

#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "Logger.h"

#define DEF_ROLLSIZE 1024*1024*1024
#define DEF_FLUSHINTERVAL 500

class AsyncLogging
{
public:
    typedef std::list<LoggerPtr> LoggerList;
    AsyncLogging(const std::string & basename,
                 Logger::LogLevel level,
                 size_t rollSize = DEF_ROLLSIZE,
                 int flushInterval = DEF_FLUSHINTERVAL);

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
    uint32_t _flushInterval;
    bool _running;

    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;

    LoggerList _loggers;


};

#endif // _ASYNC_LOGGING_H
