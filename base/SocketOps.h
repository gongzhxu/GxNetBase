#ifndef _SOCKET_OPT_H_
#define _SOCKET_OPT_H_

#define _CRT_SECURE_NO_DEPRECATE	// remove warning C4996,

#include <arpa/inet.h>
#include <string>
#include "ConnInfo.h"

namespace base
{

union SockAddr
{
    struct sockaddr_in addr;
    struct sockaddr_in6 addr6;
};

int setAddr(int sa_family, const char * ip, const uint16_t port, base::SockAddr * pAddr);
void getLocalAddr(int sockfd, std::string & ip, uint16_t & port);
void getPeerAddr(int sockfd, std::string & ip, uint16_t & port);
void setReuseAddr(int sockfd, bool on);
void setReusePort(int sockfd, bool on);
void setTcpNoDely(int sockfd, bool on);
void setKeepAlive(int sockfd, bool on, int keepIdle = 60, int keepInterval = 5, int keepCount = 3);
bool isZeroAddr(int sa_family, const char * ip);
std::string getHostMac();
std::string getHostName();
void getAddrInfo(std::vector<AddrInfo> & addrInfos, uint32_t port = 0);

}




#endif // _SOCKET_OPT_H_
