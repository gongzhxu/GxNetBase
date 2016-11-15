#ifndef _FILE_OPS_H_
#define _FILE_OPS_H_

#include <string>

namespace base
{
bool isDir(const std::string & path);
bool isFile(const std::string & path);
bool isSpecialDir(const std::string & path);
void rmFile(const std::string & path);
std::string getPwd();
}

#endif // _FILE_OPS_H_
