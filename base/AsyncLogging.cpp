#include "AsyncLogging.h"

#include <assert.h>
#include <chrono>

#include "LogFile.h"
#include "ConfigFileReader.h"

AsyncLogging::AsyncLogging(const std::string & basename,
                           Logger::LogLevel level,
                           size_t rollSize,
                           int flushInterval):
     _basename(basename),
     _level(level),
     _rollSize(rollSize),
     _flushInterval(flushInterval),
     _running(false)
{
    _thread = std::thread(std::bind(&AsyncLogging::threadFunc, this));
}

AsyncLogging::AsyncLogging(const char * szCfgFile):
    _basename("default"),
    _level(Logger::INFO),
    _rollSize(DEF_ROLLSIZE),
    _flushInterval(DEF_FLUSHINTERVAL),
    _running(false)
{
    ConfigFileReader cfgFile(szCfgFile);
    char * strBaseName = cfgFile.GetConfigName("Name");
    if(strBaseName)
    {
        _basename = strBaseName;
    }

    char * strLevel = cfgFile.GetConfigName("Level");
    if(strLevel)
    {
        _level = static_cast<Logger::LogLevel>(atoi(strLevel));
    }

    char * strRollSize = cfgFile.GetConfigName("RollSize");
    if(strRollSize)
    {
        _rollSize = atol(strRollSize);
    }

    char * strFlushInterval = cfgFile.GetConfigName("FlushInterval");
    if(strFlushInterval)
    {
        _flushInterval = atoi(strFlushInterval);
        if(_flushInterval < DEF_FLUSHINTERVAL)
        {
            _flushInterval = DEF_FLUSHINTERVAL;
        }
    }

    _thread = std::thread(std::bind(&AsyncLogging::threadFunc, this));
}

AsyncLogging::~AsyncLogging()
{
    if(_running)
    {
        _running = false;
        _cond.notify_one();
        _thread.join();
    }
    else
    {
        _thread.detach();
    }
}

void AsyncLogging::append(LoggerPtr && logger)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _loggers.push_back(std::move(logger));
    }

    _cond.notify_one();
}

void AsyncLogging::threadFunc()
{
    _running = true;
    LogFile output(_basename, _rollSize, false);
    while(true)
    {
        LoggerList loggers;

        {
            std::unique_lock<std::mutex> lock(_mutex);

            while(_loggers.empty())
            {
                if(!_running)
                {
                    return;
                }

                output.append(NULL, 0);
                _cond.wait_for(lock, std::chrono::milliseconds(_flushInterval));
            }

            loggers.swap(_loggers);
        }

        for(LoggerList::iterator it = loggers.begin(); it != loggers.end(); ++it)
        {
            char data[Logger::MAX_LOG_LEN];

            LoggerPtr pLogger = *it;
            size_t len = pLogger->format(data, Logger::MAX_LOG_LEN);
            printf(data);
            output.append(data, len);
        }

        fflush(stdout);
    }
}
