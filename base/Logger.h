#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string.h>
#include <memory>

#include "TimeStamp.h"


class SourceFile
{
public:
    template<int N>
    inline SourceFile(const char (&arr)[N]):
        data_(arr),
        size_(N-1)
    {
        const char* slash = strrchr(data_, '/');
        if(slash)
        {
            data_ = slash + 1;
            size_ -= static_cast<int>(data_ - arr);
        }
        else
        {
            slash = strrchr(data_, '\\');
            if(slash)
            {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }
    }

    explicit SourceFile(const char * filename)
        :data_(filename)
    {
        const char* slash = strrchr(filename, '/');
        if(slash)
        {
            data_ = slash + 1;
            size_ = static_cast<int>(strlen(data_));
        }
        else
        {
            slash = strrchr(filename, '\\');
            if(slash)
            {
                data_ = slash + 1;
                size_ = static_cast<int>(strlen(data_));
            }
        }
    }

    const char * data() { return data_; }
private:
    const char * data_;
    int size_;
};

class Logger;
typedef std::shared_ptr<Logger> LoggerPtr;
#define MakeLoggerPtr std::make_shared<Logger>

class Logger
{
public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LEVELS
    };

    Logger(const char * fmt, ...);
    Logger(LogLevel level, const char * file, int line, const char * func, const char * fmt, ...);
    ~Logger();

    LogLevel level() const { return level_; }
    bool raw() { return raw_; }
    size_t format(char * data, size_t len);
    void format(std::string & data);
private:
    void formatTime();

private:
    LogLevel    level_;
    int         tid_;
    SourceFile  file_;
    int         line_;
    const char * func_;
    char        time_[64];
    std::string content_;
    bool        raw_;
};

#endif // _LOGGER_H
