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
#include "BaseUtil.h"

#define DEF_ROLLSIZE 1024*1024*1024
#define DEF_FLUSHINTERVAL 1000
#define DEF_AUTORM 15
#define DAYILY_SECONDS 24*60*60

class LogFile
{
public:
    LogFile();

    void append(const char * logline);
    void append(const char * logline, int len);

    void setLogFolder(const std::string & logFolder) { _logFolder = logFolder; }
    void setBaseName(const std::string & baseName) { _baseName = baseName; }
    void setRollSize(size_t rollSize) { _rollSize = rollSize; }
    void setFlushInterval(int flushInterval) { _flushInterval = flushInterval; }
    void setAutoRm(int autoRm) { _autoRm = autoRm; }
private:
    void rollFile(const time_t & time);
    void rmFile(const time_t & time);
    std::string getLogFileName(const std::string & basename, const time_t & time);
    std::string getLogPath(const time_t & time);

    class File
    {
    public:
        File(std::string & filename):
            _filename(filename),
            _fp(::fopen(filename.c_str(), "a")),
            _writtenBytes(0)
            {
                if(!_fp)
                {
                    ABORT_MSG("create logfile=%s,error=%s\n", filename.c_str(), strerror(errno));
                }
            }

        ~File() { ::fclose(_fp); }
    public:
        size_t getWrittenBytes() { return _writtenBytes; }

        size_t fwrite(const char * logline, const size_t len)
        {
            _fp = ::freopen(_filename.c_str(), "a", _fp);
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
    std::string _logFolder;
    std::string _baseName;
    size_t      _rollSize;
    int         _flushInterval;
    int         _autoRm;

    time_t _lastFlush;
    int _fileDay;

    std::unique_ptr<File> _file;
};

#endif // _LOGFILE_H
