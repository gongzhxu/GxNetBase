#include "TimeStamp.h"
#include "StringOps.h"

#include <sys/time.h>

std::string TimeStamp::format() const
{
    std::string strTime;

    time_t seconds = this->seconds() + 8*3600;
    int microseconds = this->microseconds()%(1000*1000);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    base::sprintfex(strTime, "%4d%02d%02d %02d:%02d:%02d.%06d",
                        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
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



