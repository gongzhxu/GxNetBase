#include "LogFile.h"

#include <unistd.h>
#include <limits.h>

#include "FileOps.h"

LogFile::LogFile(const std::string & basename,
                 size_t rollSize,
                 int flushInterval,
                 int autoRm):
    _basename(basename),
    _rollSize(rollSize),
    _flushInterval(flushInterval),
    _autoRm(autoRm),
    _lastFlush(0)
{
    rmFile();
    rollFile();
}

void LogFile::append(const char * logline)
{
    append(logline, strlen(logline));
}

void LogFile::append(const char * logline, int len)
{
    time_t now = ::time(NULL);
    struct tm * pTm = localtime(&now);
    int nowDay = pTm->tm_mday;
    if(len > 0)
    {
        _file->fwrite(logline, len);
    }

    if(_file->getWrittenBytes() > _rollSize || _fileDay != nowDay)
    {
        rmFile();
        rollFile();
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

void LogFile::rollFile()
{
    time_t now = time(NULL);
    std::string filename = getLogFileName(_basename, now);

    _file.reset(new File(filename));

    struct tm * pTm = localtime(&now);
    _fileDay = pTm->tm_mday;
}

void LogFile::rmFile()
{
    time_t now = time(NULL) - _autoRm;
    std::string logPath = getLogPath(now);

    base::rmFile(logPath.c_str());
}

std::string LogFile::getLogFileName(const std::string & basename, time_t & now)
{
    std::string filename;

    filename = base::getPwd() += "log/";
    mkdir(filename.c_str(), 0744);

    char timebuf[32] = {0};
    struct tm tm;
    localtime_r(&now, &tm);

    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d/", &tm);
    filename += timebuf;
    mkdir(filename.c_str(), 0744);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);

    filename += basename + timebuf + ".log";
    return filename;
}

std::string LogFile::getLogPath(time_t & now)
{
    std::string logPath;

    char timebuf[32] = {0};
    struct tm tm;
    localtime_r(&now, &tm);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d/", &tm);

    logPath = base::getPwd() + "log/" + timebuf;
    return logPath;
}

