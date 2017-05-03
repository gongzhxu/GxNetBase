#ifndef _TIME_STAMP_
#define _TIME_STAMP_

#include <functional>
#include <string>

class TimeStamp:public std::less_equal<TimeStamp>
{
public:
    static const int MicroSecondsPerSecond = 1000*1000;

    TimeStamp(int64_t ms):ms_(ms)
    {}

    int64_t microseconds() const
    {
        return ms_;
    }

    int64_t milliseconds() const
    {
        return ms_/1000;
    }

    time_t seconds() const
    {
        return static_cast<time_t>(ms_ / MicroSecondsPerSecond);
    }

    std::string format() const;

    static TimeStamp now();
    static time_t time();

private:
    int64_t ms_;
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
