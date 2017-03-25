#include "BaseUtil.h"

#include <vector>
#include <unistd.h>

#define MAX_WRITE_INFO 32

void writeinfo(const char * action, uint32_t id, const char * hostname, const char * version, const char * productid)
{
    std::vector<std::string> strMsgs;

    {
        std::string strMsg;
        base::sprintfex(strMsg, "%s - action=%s, pid=%d, id=%d, hostname=%s, version=%s, productid=%s\n",
                                        TimeStamp::now().format().c_str(),
                                        action,
                                        getpid(),
                                        id,
                                        hostname,
                                        version,
                                        productid);

        strMsgs.emplace_back(strMsg);
        FILE * fp = fopen("server.pid", "r");
        if(fp)
        {
            for(size_t i = 0; i < MAX_WRITE_INFO-1 && !feof(fp); ++i)
            {
                char szMsg[1024] = {0};
                fgets(szMsg, sizeof(szMsg), fp);
                strMsgs.emplace_back(szMsg);
            }

            fclose(fp);
        }
    }

    {
        FILE * fp = fopen("server.pid", "w");
        if(!fp)
        {
            ABORT_MSG("open file=%s,error=%s\n", "server.pid", strerror(errno));
            return;
        }

        for(size_t i = 0; i < strMsgs.size(); ++i)
        {
            fwrite(strMsgs[i].c_str(), strMsgs[i].size(), sizeof(char), fp);
        }

        fclose(fp);
    }
}

AsyncLogging & getLogger()
{
    //log instance
    static AsyncLogging log("log.conf");
    return log;
}

