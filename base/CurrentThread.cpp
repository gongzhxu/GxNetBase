#include "CurrentThread.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

//thread value
__thread int CurrentThread::t_cachedTid = 0;

static int gettid()
{
    return static_cast<int>(::syscall(SYS_gettid));
}

void CurrentThread::cacheTid()
{
    if(t_cachedTid == 0)
    {
        t_cachedTid = gettid();
    }
}


