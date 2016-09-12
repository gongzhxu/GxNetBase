#include "BaseUtil.h"

#include <unistd.h>

void writePid()
{
	uint32_t curPid;

	curPid = getpid();

    FILE* f = fopen("server.pid", "w");
    assert(f);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d\n", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}

AsyncLogging & getLogger()
{
    static AsyncLogging log("log.conf");
    return log;
}

void sprintfex(std::string & str, const char * format, ...)
{

    int len = 0;
    va_list arglist1;
    va_start(arglist1, format);

    str.resize(256);
    len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist1);
    if(static_cast<size_t>(len) > str.size())
    {
        str.resize(len+1);

        va_list arglist2;
        va_start(arglist2, format);
        len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist2);
        va_end(arglist2);
    }

    if(len >= 0)
    {
        str.resize(len);
    }

    va_end(arglist1);
}

