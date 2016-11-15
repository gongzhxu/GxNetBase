#include "AsyncLogging.h"

#include <assert.h>
#include <chrono>

#include "LogFile.h"
#include "ConfigReader.h"
#include "Buffer.h"

#define DEF_ROLLSIZE 1024*1024*1024
#define DEF_FLUSHINTERVAL 500
#define DEF_AUTORM 15
#define DAYILY_SECONDS 24*60*60
#define MAX_LOG_BUF_SIZE 1024000

AsyncLogging::AsyncLogging(const char * szCfgFile):
    _running(true)
{
    ConfigReader cfgFile(szCfgFile);

    _basename = cfgFile.GetNameStr("Name", "default");
    _level = static_cast<Logger::LogLevel>(cfgFile.GetNameInt("Level", Logger::INFO));
    _rollSize = cfgFile.GetNameInt("RollSize", DEF_ROLLSIZE);
    _flushInterval = cfgFile.GetNameInt("FlushInterval", DEF_FLUSHINTERVAL);
    if(_flushInterval < DEF_FLUSHINTERVAL)
    {
        _flushInterval = DEF_FLUSHINTERVAL;
    }
    _autoRm = cfgFile.GetNameInt("AutoRm", DEF_AUTORM)*DAYILY_SECONDS;//use second
    if(_autoRm < 0)
    {
        _autoRm = DEF_AUTORM*DAYILY_SECONDS;
    }

    _print = cfgFile.GetNameInt("Print", true);

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
    LogFile output(_basename, _rollSize, _flushInterval, _autoRm);

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
