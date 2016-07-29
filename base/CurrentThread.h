#ifndef _CURRENT_THREAD_H_
#define _CURRENT_THREAD_H_

struct CurrentThread
{

static void cacheTid();

static inline int tid()
{
    if(__builtin_expect(t_cachedTid == 0, 0))
    {
        cacheTid();
    }
    return t_cachedTid;
}

static __thread int t_cachedTid;

};


#endif
