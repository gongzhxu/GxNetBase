#include "LogFile.h"

#include <unistd.h>
#include <limits.h>

#include "FileOps.h"
#include "TimeStamp.h"

LogFile::LogFile(const std::string & logFolder,
                 const std::string & baseName,
                 size_t rollSize,
                 int flushInterval,
                 int autoRm):
    _logFolder(logFolder),
    _baseName(baseName),
    _rollSize(rollSize),
    _flushInterval(flushInterval),
    _autoRm(autoRm),
    _lastFlush(0)
{
    time_t now = time(NULL) + 8*3600;

    rmFile(now);
    rollFile(now);
}

void LogFile::append(const char * logline)
{
    append(logline, strlen(logline));
}

void LogFile::append(const char * logline, int len)
{
    time_t now = time(NULL) + 8*3600;

    struct tm  tm_time;
    gmtime_r(&now, &tm_time);
    int nowDay = tm_time.tm_mday;
    if(len > 0)
    {
        _file->fwrite(logline, len);
    }

    if(_file->getWrittenBytes() > _rollSize || _fileDay != nowDay)
    {
        rmFile(now);
        rollFile(now);
    }
    else
    {
        if(now - _lastFlush > _flushInterval)
        {
            _lastFlush = now;
            _file->fflush();
        }
    }
}

void LogFile::rollFile(const time_t & time)
{
    std::string filename = getLogFileName(_baseName, time);

    _file.reset(new File(filename));

    struct tm  tm_time;
    gmtime_r(&time, &tm_time);

    _fileDay = tm_time.tm_mday;
}

void LogFile::rmFile(const time_t & time)
{
    std::string logPath = getLogPath(time - _autoRm);
    base::rmFile(logPath.c_str());
}

std::string LogFile::getLogFileName(const std::string & basename, const time_t & time)
{
    std::string filename;

    filename = base::getPwd() + _logFolder + "/";
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
    struct tm tm_time;
    gmtime_r(&time, &tm_time);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d/", &tm_time);

    logPath = base::getPwd() + _logFolder + "/" + timebuf;
    return logPath;
}

