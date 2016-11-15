#include "FileOps.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "StringOps.h"

bool base::isDir(const std::string & path)
{
    struct stat statbuf;
    if(lstat(path.c_str(), &statbuf) == 0)
    {
        return S_ISDIR(statbuf.st_mode) != 0;
    }
    return false;
}

bool base::isFile(const std::string & path)
{
    struct stat statbuf;
    if(lstat(path.c_str(), &statbuf) == 0)
    {
        return S_ISREG(statbuf.st_mode) != 0;
    }

    return false;
}

bool base::isSpecialDir(const std::string & path)
{
    return path == "." || path == "..";
}

void base::rmFile(const std::string & path)
{
    if(isFile(path))
    {
        remove(path.c_str());
        return;
    }

    if(isDir(path))
    {
        DIR * ptrDir = nullptr;
        dirent * dirInfo = nullptr;

        if((ptrDir = opendir(path.c_str())) == NULL)
        {
            return;
        }

        while((dirInfo = readdir(ptrDir)) != NULL)
        {
            std::string subPath;
            base::sprintfex(subPath, "%s/%s", path.c_str(), dirInfo->d_name);
            if(isSpecialDir(dirInfo->d_name))
            {
                continue;
            }

            rmFile(subPath);
            rmdir(subPath.c_str());
        }
        rmdir(path.c_str());
        closedir(ptrDir);
    }
}

std::string base::getPwd()
{
    char path[PATH_MAX] = {0};
    int len = readlink("/proc/self/exe", path, PATH_MAX);
    if(len < 0 || (len >= PATH_MAX))
    {
        return "";
    }

    path[len] = '\0';
    for(int i = len; i >= 0; --i)
    {
        if(path[i] == '/')
        {
            path[i + 1] = '\0';
            break;
        }
    }
    return path;
}

