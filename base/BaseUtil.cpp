#include "BaseUtil.h"

#include <unistd.h>

AsyncLogging g_baselog("log.conf");

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
