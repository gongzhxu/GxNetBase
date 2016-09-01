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
        _data(arr),
        _size(N-1)
    {
        const char * slash = strrchr(_data, '/'); // builtin function
        if(slash)
        {
            _data = slash + 1;
            _size -= static_cast<int>(_data - arr);
        }
    }

    explicit SourceFile(const char * filename)
        :_data(filename)
    {
        const char* slash = strrchr(filename, '/');
        if(slash)
        {
            _data = slash + 1;
        }
        else
        {
            slash = strrchr(filename, '\\');
            _data = slash + 1;
        }

        _size = static_cast<int>(strlen(_data));
    }

    const char * data() { return _data; }
private:
    const char * _data;
    int _size;
};

class Logger;
typedef std::shared_ptr<Logger> LoggerPtr;
#define MakeLoggerPtr std::make_shared<Logger>

class Logger
{
public:
    static const int MAX_LOG_LEN = 2048;
    static const int MAX_CONTENT_LEN = 1024;

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

    Logger(LogLevel level, const char * file, int line, const char * func, const char * fmt, ...);
    ~Logger();

    LogLevel level() const { return _level; }
    size_t format(char * data, size_t len);
private:
    void formatTime();

private:
    LogLevel    _level;
    int         _tid;
    SourceFile  _file;
    int         _line;
    const char * _func;
    char        _time[64];
    char        _content[MAX_CONTENT_LEN];
};

#endif // _LOGGER_H
