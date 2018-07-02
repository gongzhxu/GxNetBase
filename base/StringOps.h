#ifndef _STRING_OPT_H_
#define _STRING_OPT_H_

#include <string>
#include <vector>
#include <inttypes.h>

#define SAFE_A2A(data) (data?data:"")
#define SAFE_A2I(data) (data?atoi(data):0)
#define SAFE_A2L(data) (data?atol(data):0)

namespace base
{

void sprintfex(std::string & str, const char * format, ...);
std::string strprintfex(const char * format, ...);
void vsprintfex(std::string & str, const char * format, va_list arglist);
std::string vstrprintfex(const char * format, va_list arglist);
void splitex(const std::string  & str, const std::string delim, std::vector<std::string> & ret);
std::string splitex(const std::string & str, const std::string delim, int n);

inline const char * getformat(int8_t) { return "%d"; }
inline const char * getformat(uint8_t) { return "%u"; }
inline const char * getformat(int16_t) { return "%d"; }
inline const char * getformat(uint16_t) { return "%u"; }
inline const char * getformat(int32_t) { return "%d"; }
inline const char * getformat(uint32_t) { return "%u"; }
inline const char * getformat(int64_t) { return "%lld"; }
inline const char * getformat(uint64_t) { return "%llu"; }

template<typename T>
std::string toString(const T data)
{
    char szData[32] = {0};
    snprintf(szData, sizeof(szData), base::getformat(data), data);
    return szData;
}

std::string utf8_substr(const std::string & str, size_t pos, size_t len);
}

#endif // _STRING_OPT_H_
