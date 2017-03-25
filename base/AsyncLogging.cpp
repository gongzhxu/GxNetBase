#include "AsyncLogging.h"

#include <chrono>

#include "LogFile.h"
#include "ConfigReader.h"
#include "Buffer.h"

#define DEF_ROLLSIZE 1024*1024*1024
#define DEF_FLUSHINTERVAL 1000
#define DEF_AUTORM 15
#define DAYILY_SECONDS 24*60*60
#define MAX_LOG_BUF_SIZE 1024000

AsyncLogging::AsyncLogging(const char * fileName):
    _running(true)
{
    //read the configuration item from config file
    ConfigReader cfgFile(fileName);
    _logFolder = cfgFile.GetNameStr("Folder", "log");
    _baseName = cfgFile.GetNameStr("Name", "default");
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

    //start the thread
    _thread = std::thread(std::bind(&AsyncLogging::threadFunc, this));
}

AsyncLogging::~AsyncLogging()
{
    //stop the running thread
    _running = false;
    _cond.notify_one();
    _thread.join();
}

void AsyncLogging::append(LoggerPtr && logger)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _loggers.emplace_back(std::move(logger));
    }

    _cond.notify_one();
}

void AsyncLogging::threadFunc()
{
    //make LogFile object to handle the logger
    LogFile output(_logFolder, _baseName, _rollSize, (_flushInterval+1000)/1000, _autoRm);

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


        Buffer outputBuf; // the file log buffer
        Buffer printBuf; // the screen print buffer
        std::string data;

        for(auto it = loggers.begin(); it != loggers.end();)
        {
            LoggerPtr pLogger = *(it++);
            pLogger->format(data);

            outputBuf.append(data.c_str(), data.size());
            if((_print && pLogger->level() == Logger::INFO) || pLogger->raw())
            {
                printBuf.append(data.c_str(), data.size());
            }

            //write the log buffer to destination
            if(outputBuf.size() > MAX_LOG_BUF_SIZE || it == loggers.end())
            {
                output.append(outputBuf.data(), outputBuf.size());
                outputBuf.clear();
            }

            if(printBuf.size() > MAX_LOG_BUF_SIZE || it == loggers.end())
            {
                ::fwrite(printBuf.data(), sizeof(char), printBuf.size(), stdout);
                printBuf.clear();
            }
        }
    }
}
