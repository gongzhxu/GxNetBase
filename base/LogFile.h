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
            m_fp(::fopen(filename.c_str(), "ae")),
            m_writtenBytes(0)
            {
                if(m_fp == NULL)
                {
                    fprintf(stderr,
                            "create log file %s error:%s\n",
                            filename.c_str(),
                            strerror(errno));
                    abort();
                }
            }

        ~File() { ::fclose(m_fp); }
    public:
        size_t getWrittenBytes() { return m_writtenBytes; }

        size_t fwrite(const char * logline, const size_t len)
        {
            m_writtenBytes += len;
            return ::fwrite(logline, 1, len, m_fp);
        }

        void fflush()
        {
            ::fflush(m_fp);
        }
    private:
        FILE * m_fp;
        size_t m_writtenBytes;
    };

private:
    const std::string m_basename;
    const size_t m_rollSize;
    const int m_flushInterval;
    time_t m_lastFlush;
    int m_fileDay;

    std::unique_ptr<File> m_file;
};

#endif // _LOGFILE_H
