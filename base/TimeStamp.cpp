#include "TimeStamp.h"
#include "StringOps.h"

#include <sys/time.h>

std::string TimeStamp::format() const
{
    std::string strTime;

    time_t seconds = this->seconds() + 8*3600;
    int microseconds = this->microseconds()%(1000*1000);
    struct tm tmtime_;
    gmtime_r(&seconds, &tmtime_);
    base::sprintfex(strTime, "%4d%02d%02d %02d:%02d:%02d.%06d",
                        tmtime_.tm_year + 1900, tmtime_.tm_mon + 1, tmtime_.tm_mday,
                        tmtime_.tm_hour, tmtime_.tm_min, tmtime_.tm_sec, microseconds);
    return strTime;
}

TimeStamp TimeStamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return TimeStamp(tv.tv_sec*MicroSecondsPerSecond + tv.tv_usec);
}

time_t TimeStamp::time()
{
    return ::time(nullptr);
}



