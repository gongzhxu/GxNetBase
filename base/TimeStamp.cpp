#include "TimeStamp.h"

#include <sys/time.h>

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
