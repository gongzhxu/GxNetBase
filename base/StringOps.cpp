#include "StringOps.h"

#include <stdarg.h>

void base::sprintfex(std::string & str, const char * format, ...)
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

void base::splitex(const std::string & str, const std::string delim, std::vector<std::string> & ret)
{
    size_t last = 0;
    size_t index=str.find_first_of(delim,last);
    while(index != std::string::npos)
    {
        std::string str1 = str.substr(last, index-last);
        if(!str1.empty())
        {
            ret.push_back(str1);
        }

        last=index+1;
        index=str.find_first_of(delim,last);
    }

    if(index-last > 0)
    {
        std::string str1 = str.substr(last, index-last);
        if(!str1.empty())
        {
            ret.push_back(str1);
        }
    }
}
