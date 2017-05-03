#include "LogFile.h"

#include <unistd.h>
#include <limits.h>

#include "FileOps.h"
#include "TimeStamp.h"

LogFile::LogFile():
    logFolder_("log"),
    baseName_("default"),
    rollSize_(DEF_ROLLSIZE),
    flushInterval_(DEF_FLUSHINTERVAL),
    autoRm_(DEF_AUTORM*DAYILY_SECONDS),
    lastFlush_(0)
{

}

void LogFile::append(const char * logline)
{
    append(logline, strlen(logline));
}

void LogFile::append(const char * logline, int len)
{
    time_t now = time(NULL) + 8*3600;

    struct tm  tmtime_;
    gmtime_r(&now, &tmtime_);
    int nowDay = tmtime_.tm_mday;
    if(fileDay_ != nowDay || !fileObj_ || fileObj_->getWrittenBytes() + len > rollSize_)
    {
        rmFile(now);
        rollFile(now);
    }

    if(len > 0)
    {
        fileObj_->fwrite(logline, len);
    }

    if(now - lastFlush_ > flushInterval_)
    {
        lastFlush_ = now;
        fileObj_->fflush();
    }
}

void LogFile::rollFile(const time_t & time)
{
    std::string filename = getLogFileName(baseName_, time);

    fileObj_.reset(new File(filename));

    struct tm  tmtime_;
    gmtime_r(&time, &tmtime_);

    fileDay_ = tmtime_.tm_mday;
}

void LogFile::rmFile(const time_t & time)
{
    std::string logPath = getLogPath(time - autoRm_);
    base::rmFile(logPath.c_str());
}

std::string LogFile::getLogFileName(const std::string & basename, const time_t & time)
{
    std::string filename;

    filename = base::getPwd() + logFolder_ + "/";
    mkdir(filename.c_str(), 0755);

    char timebuf[32] = {0};
    struct tm tm;
    gmtime_r(&time, &tm);

    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d/", &tm);
    filename += timebuf;
    mkdir(filename.c_str(), 0755);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);

    filename += basename + timebuf + ".log";
    return filename;
}

std::string LogFile::getLogPath(const time_t & time)
{
    std::string logPath;

    char timebuf[32] = {0};
    struct tm tmtime_;
    gmtime_r(&time, &tmtime_);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d/", &tmtime_);

    logPath = base::getPwd() + logFolder_ + "/" + timebuf;
    return logPath;
}

