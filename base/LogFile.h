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

#include "FileOps.h"

#define DEF_ROLLSIZE 1024*1024*1024
#define DEF_FLUSHINTERVAL 1
#define DEF_AUTORM 15
#define DAYILY_SECONDS 24*60*60

class LogFile
{
public:
    LogFile();

    void append(const char * logline);
    void append(const char * logline, int len);

    void setLogFolder(const std::string & logFolder) { logFolder_ = logFolder; }
    void setBaseName(const std::string & baseName) { baseName_ = baseName; }
    void setRollSize(size_t rollSize) { rollSize_ = rollSize; }
    void setFlushInterval(int flushInterval) { flushInterval_ = flushInterval; }
    void setAutoRm(int autoRm) { autoRm_ = autoRm; }
private:
    void rollFile(const time_t & time);
    void rmFile(const time_t & time);
    std::string getLogFileName(const std::string & basename, const time_t & time);
    std::string getLogPath(const time_t & time);

    class File
    {
    public:
        File(std::string & filename):
            filename_(filename),
            fp_(::fopen(filename_.c_str(), "a")),
            writtenBytes_(0)
            {
                if(!fp_) fprintf(stderr, "create logfile=%s,error=%s\n", filename_.c_str(), strerror(errno));
            }

        ~File() { fclose(); }
    public:
        size_t getWrittenBytes() { return writtenBytes_; }

        size_t fwrite(const char * logline, const size_t len)
        {
            if(!fopen())
            {
                return 0;
            }

            writtenBytes_ += len;
            return ::fwrite(logline, 1, len, fp_);
        }

        void fflush()
        {
            if(fp_) ::fflush(fp_);
        }

        void fclose()
        {
            if(fp_) ::fclose(fp_);
        }

        bool fopen()
        {
            if(!fp_ || !base::isFile(filename_))
            {
                fclose();
                fp_ = ::fopen(filename_.c_str(), "a");
            }

            return fp_ != nullptr;
        }
    private:
        std::string filename_;
        FILE * fp_;
        size_t writtenBytes_;
    };

private:
    std::string logFolder_;
    std::string baseName_;
    size_t      rollSize_;
    int         flushInterval_;
    int         autoRm_;

    time_t lastFlush_;
    int fileDay_;

    std::unique_ptr<File> fileObj_;
};

#endif // _LOGFILE_H
