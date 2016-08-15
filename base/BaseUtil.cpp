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

    {
        va_list arglist;
        va_start(arglist, format);
        len = vsnprintf(0, 0, format, arglist);
        va_end(arglist);
    }

    {
        va_list arglist;
        va_start(arglist, format);
        if(len)
        {
            str.resize(len+1);
            vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist);
        }
        va_end(arglist);
    }

}
