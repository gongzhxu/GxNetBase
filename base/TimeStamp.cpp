#include "TimeStamp.h"

#include <sys/time.h>

TimeStamp TimeStamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return TimeStamp(tv.tv_sec*MicroSecondsPerSecond + tv.tv_usec);
}
