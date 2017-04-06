#include "AsyncLogging.h"

#include <chrono>

#include "LogFile.h"
#include "ConfigReader.h"
#include "Buffer.h"

#define MAX_LOG_BUF_SIZE 1024000

AsyncLogging::AsyncLogging(const char * fileName):
    _running(true),
    _output(new LogFile())
{

    loadConfig(fileName);
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

void AsyncLogging::loadConfig(const char * fileName)
{
    ConfigReader cfgFile(fileName);
    _flushInterval = cfgFile.GetNameInt("FlushInterval", DEF_FLUSHINTERVAL);
    if(_flushInterval < DEF_FLUSHINTERVAL)
    {
        _flushInterval = DEF_FLUSHINTERVAL;
    }
    _level = cfgFile.GetNameInt("Level", Logger::INFO);
    _print = cfgFile.GetNameInt("Print", true);


    std::string logFolder = cfgFile.GetNameStr("Folder", "log");
    std::string baseName = cfgFile.GetNameStr("Name", "default");
    int rollSize = cfgFile.GetNameInt("RollSize", DEF_ROLLSIZE);
    int autoRm = cfgFile.GetNameInt("AutoRm", DEF_AUTORM)*DAYILY_SECONDS;//use second
    if(autoRm < 0)
    {
        autoRm = DEF_AUTORM*DAYILY_SECONDS;
    }


    _output->setLogFolder(logFolder);
    _output->setBaseName(baseName);
    _output->setRollSize(rollSize);
    _output->setFlushInterval(_flushInterval);
    _output->setAutoRm(autoRm);
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
    while(true)
    {
        LoggerList loggers;

        {
            std::unique_lock<std::mutex> lock(_mutex);

            while(_loggers.empty())
            {
                if(!_running)
                {
                    _output->append("log thread exit!!!\n");
                    return;
                }

                _output->append(NULL, 0);
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
                _output->append(outputBuf.data(), outputBuf.size());
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
