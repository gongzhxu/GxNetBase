#include "StringOps.h"

#include <stdarg.h>

void base::sprintfex(std::string & str, const char * format, ...)
{
    int len = 0;
    va_list arglist1, arglist2;
    va_start(arglist1, format);
    va_copy(arglist2, arglist1);

    str.resize(128);
    len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist1);
    if(static_cast<size_t>(len) >= str.size())
    {
        str.resize(len+1);
        len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist2);
    }

    if(len >= 0)
    {
        str.resize(len);
    }

    va_end(arglist1);
    va_end(arglist2);
}

std::string base::strprintfex(const char * format, ...)
{
    std::string str;

    int len = 0;
    va_list arglist1, arglist2;
    va_start(arglist1, format);
    va_copy(arglist2, arglist1);

    str.resize(128);
    len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist1);
    if(static_cast<size_t>(len) >= str.size())
    {
        str.resize(len+1);
        len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist2);
    }

    if(len >= 0)
    {
        str.resize(len);
    }

    va_end(arglist1);
    va_end(arglist2);

    return str;
}

void base::vsprintfex(std::string & str, const char * format, va_list arglist)
{
    int len = 0;
    va_list arglist1;
    va_copy(arglist1, arglist);

    str.resize(128);
    len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist);
    if(static_cast<size_t>(len) >= str.size())
    {
        str.resize(len+1);
        len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist1);
    }

    if(len >= 0)
    {
        str.resize(len);
    }

    va_end(arglist1);
}

std::string base::vstrprintfex(const char * format, va_list arglist)
{
    std::string str;

    int len = 0;
    va_list arglist1;
    va_copy(arglist1, arglist);

    str.resize(128);
    len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist);
    if(static_cast<size_t>(len) >= str.size())
    {
        str.resize(len+1);
        len = vsnprintf(const_cast<char *>(str.c_str()), str.size(), format, arglist1);
    }

    if(len >= 0)
    {
        str.resize(len);
    }

    va_end(arglist1);
    return str;
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
            ret.emplace_back(str1);
        }

        last=index+1;
        index=str.find_first_of(delim,last);
    }

    std::string str1 = str.substr(last);
    if(!str1.empty())
    {
        ret.emplace_back(str1);
    }
}

std::string base::splitex(const std::string & str, const std::string delim, int n)
{
    std::string ret;
    if(n > 0)
    {
        int i = 0;
        size_t last = 0;
        size_t index=str.find_first_of(delim,last);
        while(index != std::string::npos)
        {
            if(++i == n)
            {
                ret = str.substr(last, index-last);
                break;
            }

            last=index+1;
            index=str.find_first_of(delim,last);
        }

        if(++i == n)
        {
            ret = str.substr(last);
        }
    }
    else if(n < 0)
    {
        int i = 0;
        size_t last = str.size();
        size_t index=str.find_last_of(delim,last);
        while(index != std::string::npos)
        {
            if(++i == -n)
            {
                ret = str.substr(index+1, last-index);
                break;
            }

            last=index-1;
            index=str.find_last_of(delim,last);
        }

        if(++i == -n)
        {
            ret = str.substr(0, last-1);
        }
    }

    return ret;
}

std::string base::utf8_substr(const std::string & str, size_t pos, size_t len)
{
    if(len == 0)
    {
        return "";
    }

    size_t i, ix, q, min = std::string::npos, max = std::string::npos;
    for(q = 0, i = 0, ix = str.length(); i < ix; i++, q++)
    {
        if(q == pos)
        {
            min = i;
        }

        if(q <= pos + len || len == std::string::npos)
        {
            max = i;
        }

        unsigned char c = (unsigned char)str[i];

        if (c > 0 && c <= 127) i += 0;
        else if ((c & 0xE0) == 0xC0) i += 1;
        else if ((c & 0xF0) == 0xE0) i += 2;
        else if ((c & 0xF8) == 0xF0) i += 3;
        else if ((c & 0xFC) == 0xF8) i += 4;
        else return "";
    }

    if(q <= pos + len || len == std::string::npos) { max = i; }
    if(min == std::string::npos || max == std::string::npos) { return "";}

    return str.substr(min, max - min);
}


/*
const char * base::getformat(uint8_t)
{
    return "%" PRIu8;
}

const char * base::getformat(int16_t)
{
    return "%" PRId16;
}

const char * base::getformat(uint16_t)
{
    return "%" PRIu16;
}

const char * base::getformat(int32_t)
{
    return "%" PRId32;
}

const char * base::getformat(uint32_t)
{
    return "%" PRIu32;
}

const char * base::getformat(int64_t)
{
    return "%" PRId64;
}

const char * base::getformat(uint64_t)
{

}*/
