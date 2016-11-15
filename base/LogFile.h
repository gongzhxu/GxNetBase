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
            int flushInterval,
            int autoRm);
public:
    void append(const char * logline);
    void append(const char * logline, int len);

private:
    void rollFile();
    void rmFile();
    std::string getLogFileName(const std::string & basename, time_t & now);
    std::string getLogPath(time_t & now);

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
                            "create logfile=%s,error=%s\n",
                            filename.c_str(),
                            strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }

        ~File() { ::fclose(_fp); }
    public:
        size_t getWrittenBytes() { return _writtenBytes; }

        size_t fwrite(const char * logline, const size_t len)
        {
            ::freopen(_filename.c_str(), "a", _fp);
            _writtenBytes += len;
            return ::fwrite(logline, 1, len, _fp);
        }

        int ferror() { return ::ferror(_fp); }

        void fflush()
        {
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
    const int _autoRm;
    time_t _lastFlush;
    int _fileDay;

    std::unique_ptr<File> _file;
};

#endif // _LOGFILE_H
