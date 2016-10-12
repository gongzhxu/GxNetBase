#ifndef _STRING_OPT_H_
#define _STRING_OPT_H_

#include <string>
#include <vector>
#include <inttypes.h>

#define SAFE_A2I(data) (data?atoi(data):-1)
#define SAFE_A2L(data) (data?atol(data):-1)



namespace base
{

void sprintfex(std::string & str, const char * format, ...);
void splitex(const std::string  & str, const std::string delim, std::vector<std::string> & ret);

const char * getformat(int8_t);
const char * getformat(uint8_t);
const char * getformat(int16_t);
const char * getformat(uint16_t);
const char * getformat(int32_t);
const char * getformat(uint32_t);
const char * getformat(int64_t);
const char * getformat(uint64_t);

template<typename T>
std::string toString(const T data)
{
    char szData[32] = {0};
    snprintf(szData, sizeof(szData), base::getformat(data), data);
    return szData;
}

}

#endif // _STRING_OPT_H_
