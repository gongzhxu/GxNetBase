#include "BaseUtil.h"

#include <unistd.h>

void writeinfo(uint32_t id, const char * hostname)
{
	uint32_t curPid;

	curPid = getpid();

    FILE* f = fopen("server.pid", "w");
    assert(f);

    char szMsg[1024] = {0};

    snprintf(szMsg, sizeof(szMsg), "pid=%d, id=%d, hostname=%s\n", curPid, id, hostname);

    fwrite(szMsg, strlen(szMsg), 1, f);
    fclose(f);
}

AsyncLogging & getLogger()
{
    //log instance
    static AsyncLogging log("log.conf");
    return log;
}

