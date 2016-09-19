#ifndef _STRING_OPT_H_
#define _STRING_OPT_H_

#include <string>
#include <vector>

namespace base
{

void sprintfex(std::string & str, const char * format, ...);
void splitex(const std::string  & str, const std::string delim, std::vector<std::string> & ret);

}

#endif // _STRING_OPT_H_
