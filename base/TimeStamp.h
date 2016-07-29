#ifndef _TIME_STAMP_
#define _TIME_STAMP_

#include <functional>

class TimeStamp:public std::less_equal<TimeStamp>
{
public:
    static const int MicroSecondsPerSecond = 1000*1000;

    TimeStamp(int64_t ms):_ms(ms)
    {}

    int64_t microseconds() const
    {
        return _ms;
    }

    int64_t milliseconds() const
    {
        return _ms/1000;
    }

    time_t seconds() const
    {
        return static_cast<time_t>(_ms / MicroSecondsPerSecond);
    }

    static TimeStamp now();

private:
    int64_t _ms;
};

inline bool operator<(const TimeStamp & lhs, const TimeStamp & rhs)
{
    return lhs.microseconds() < rhs.microseconds();
}

inline bool operator==(const TimeStamp & lhs, const TimeStamp & rhs)
{
    return lhs.microseconds() == rhs.microseconds();
}

inline double timeDifference(const TimeStamp & high, const TimeStamp & low)
{
    int64_t diff = high.microseconds() - low.microseconds();
    return static_cast<double>(diff) / TimeStamp::MicroSecondsPerSecond;
}

inline TimeStamp addTime(const TimeStamp & ts, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * TimeStamp::MicroSecondsPerSecond);
    return TimeStamp(ts.microseconds() + delta);
}

#endif
