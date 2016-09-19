#include "Logger.h"

#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include "CurrentThread.h"
#include "AsyncLogging.h"

__thread char t_time[32];
__thread time_t t_lastSecond;


const char * LogLevelName[Logger::NUM_LEVELS] =
{
    "TRACE",
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "FATAL",
};

Logger::Logger(const char * fmt, ...):
    _file(""),
    _raw(true)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(_content, sizeof(_content) , fmt, args);
    va_end(args);
}

Logger::Logger(LogLevel level, const char * file, int line, const char * func, const char * fmt, ...):
    _level(level),
    _tid(CurrentThread::tid()),
    _file(file),
    _line(line),
    _func(func),
    _raw(false)
{
    formatTime();

    va_list args;
    va_start(args, fmt);
    vsnprintf(_content, sizeof(_content) , fmt, args);
    va_end(args);
}

Logger::~Logger()
{
}

void Logger::formatTime()
{
    TimeStamp ts(TimeStamp::now());

    time_t seconds = time(NULL);
    int microseconds = ts.microseconds()%(1000*1000);
    if(seconds != t_lastSecond)
    {
        t_lastSecond = seconds;
        struct tm tm_time;
        gmtime_r(&seconds, &tm_time);
        snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour + 8, tm_time.tm_min, tm_time.tm_sec);
    }

    snprintf(_time, sizeof(_time), "%s.%06d", t_time, microseconds);
}

size_t Logger::format(char * data, size_t len)
{
    if(_raw)
    {
        snprintf(data, len, "%s\n", _content);
    }
    else
    {
        snprintf(data, len, "%s [%s][%d] - %s -- <%s,%d,%s>\n",
                    _time,
                    LogLevelName[_level],
                    _tid,
                    _content,
                    _file.data(),
                    _line,
                    _func);
    }

    return strlen(data);
}

