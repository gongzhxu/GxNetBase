#include "AsyncLogging.h"

#include <chrono>

#include "LogFile.h"
#include "ConfigReader.h"
#include "Buffer.h"

#define MAX_LOG_BUF_SIZE 1024000

AsyncLogging::AsyncLogging(const char * fileName):
    running_(true),
    output_(new LogFile())
{

    loadConfig(fileName);
    //start the thread
    thread_ = std::thread(std::bind(&AsyncLogging::threadFunc, this));
}

AsyncLogging::~AsyncLogging()
{
    //stop the running thread
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLogging::loadConfig(const char * fileName)
{
    ConfigReader cfgFile(fileName);
    flushInterval_ = cfgFile.GetNameInt("FlushInterval", DEF_FLUSHINTERVAL);
    if(flushInterval_ < DEF_FLUSHINTERVAL)
    {
        flushInterval_ = DEF_FLUSHINTERVAL;
    }
    level_ = cfgFile.GetNameInt("Level", Logger::INFO);
    print_ = cfgFile.GetNameInt("Print", true);


    std::string logFolder = cfgFile.GetNameStr("Folder", "log");
    std::string baseName = cfgFile.GetNameStr("Name", "default");
    int rollSize = cfgFile.GetNameInt("RollSize", DEF_ROLLSIZE);
    int autoRm = cfgFile.GetNameInt("AutoRm", DEF_AUTORM)*DAYILY_SECONDS;//use second
    if(autoRm < 0)
    {
        autoRm = DEF_AUTORM*DAYILY_SECONDS;
    }


    output_->setLogFolder(logFolder);
    output_->setBaseName(baseName);
    output_->setRollSize(rollSize);
    output_->setFlushInterval(flushInterval_);
    output_->setAutoRm(autoRm);
}

void AsyncLogging::append(LoggerPtr && logger)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loggers_.emplace_back(std::move(logger));
    }

    cond_.notify_one();
}

void AsyncLogging::threadFunc()
{
    while(true)
    {
        LoggerList loggers;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            while(loggers_.empty())
            {
                if(!running_)
                {
                    output_->append("log thread exit!!!\n");
                    return;
                }

                output_->append(NULL, 0);
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }

            loggers.swap(loggers_);
        }


        Buffer outputBuf; // the file log buffer
        Buffer printBuf; // the screen print buffer
        std::string data;

        for(auto it = loggers.begin(); it != loggers.end();)
        {
            LoggerPtr pLogger = *(it++);
            pLogger->format(data);

            outputBuf.append(data.c_str(), data.size());
            if((print_ && pLogger->level() == Logger::INFO) || pLogger->raw())
            {
                printBuf.append(data.c_str(), data.size());
            }

            //write the log buffer to destination
            if(outputBuf.size() > MAX_LOG_BUF_SIZE || it == loggers.end())
            {
                output_->append(outputBuf.data(), outputBuf.size());
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
