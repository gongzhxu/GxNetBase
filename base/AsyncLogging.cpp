#include "AsyncLogging.h"

#include <assert.h>
#include <chrono>

#include "LogFile.h"
#include "ConfigFileReader.h"
#include "Buffer.h"

#define MAX_LOG_BUF_SIZE 1024000

AsyncLogging::AsyncLogging(const std::string & basename,
                           Logger::LogLevel level,
                           size_t rollSize,
                           int flushInterval,
                           bool print):
     _basename(basename),
     _level(level),
     _rollSize(rollSize),
     _flushInterval(flushInterval),
     _print(print),
     _running(true)
{
    _thread = std::thread(std::bind(&AsyncLogging::threadFunc, this));
}

AsyncLogging::AsyncLogging(const char * szCfgFile):
    _basename("default"),
    _level(Logger::INFO),
    _rollSize(DEF_ROLLSIZE),
    _flushInterval(DEF_FLUSHINTERVAL),
    _print(true),
    _running(true)
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

    char * strPrint = cfgFile.GetConfigName("Print");
    if(strFlushInterval)
    {
        _print = atoi(strPrint);
    }

    _thread = std::thread(std::bind(&AsyncLogging::threadFunc, this));
}

AsyncLogging::~AsyncLogging()
{
    _running = false;
    _cond.notify_one();
    _thread.join();
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
                    output.append("log thread exit!!!\n");
                    return;
                }

                output.append(NULL, 0);
                _cond.wait_for(lock, std::chrono::milliseconds(_flushInterval));
            }

            loggers.swap(_loggers);
        }


        Buffer outputBuf;
        Buffer printBuf;
        char data[Logger::MAX_LOG_LEN];

        for(auto it = loggers.begin(); it != loggers.end(); ++it)
        {
            LoggerPtr pLogger = *it;

            size_t len = pLogger->format(data, Logger::MAX_LOG_LEN);

            outputBuf.append(data, len);
            if((_print && pLogger->level() == Logger::INFO) || pLogger->raw())
            {
                printBuf.append(data, len);
            }


            if(outputBuf.size() > MAX_LOG_BUF_SIZE)
            {
                output.append(outputBuf.data(), outputBuf.size());
                outputBuf.clear();
            }

            if(printBuf.size() > MAX_LOG_BUF_SIZE)
            {
                ::fwrite(printBuf.data(), 1, printBuf.size(), stdout);
                printBuf.clear();
            }
        }

        if(!outputBuf.empty())
        {
            output.append(outputBuf.data(), outputBuf.size());
        }

        if(!printBuf.empty())
        {
            ::fwrite(printBuf.data(), 1, printBuf.size(), stdout);
        }
    }
}
