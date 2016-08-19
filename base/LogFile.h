#ifndef _LOGFILE_H_
#define _LOGFILE_H_

#include <string>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <memory>
#include <unistd.h>

class LogFile
{
public:
    LogFile(const std::string & basename,
            size_t rollSize,
            int flushInterval = 3);
public:
    void append(const char * logline, int len);

    static char * getpwd( char * buf, int len);
private:
    bool rollFile();
    std::string getLogFileName(const std::string & basename, time_t & now);

    class File
    {
    public:
        File(std::string & filename):
            _filename(filename),
            _fp(::fopen(filename.c_str(), "a")),
            _writtenBytes(0)
            {
                if(_fp == NULL)
                {
                    fprintf(stderr,
                            "create log file %s error:%s\n",
                            filename.c_str(),
                            strerror(errno));
                    abort();
                }
            }

        ~File() { ::fclose(_fp); }
    public:
        size_t getWrittenBytes() { return _writtenBytes; }

        size_t fwrite(const char * logline, const size_t len)
        {
            _writtenBytes += len;
            return ::fwrite(logline, 1, len, _fp);
        }

        int ferror() { return ::ferror(_fp); }

        void fflush()
        {
            ::freopen(_filename.c_str(), "a", _fp);
            ::fflush(_fp);
        }
    private:
        std::string _filename;
        FILE * _fp;
        size_t _writtenBytes;
    };

private:
    const std::string _basename;
    const size_t _rollSize;
    const int _flushInterval;
    time_t _lastFlush;
    int _fileDay;

    std::unique_ptr<File> _file;
};

#endif // _LOGFILE_H
