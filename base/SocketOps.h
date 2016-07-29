#ifndef _SOCKET_OPT_H_
#define _SOCKET_OPT_H_

#define _CRT_SECURE_NO_DEPRECATE	// remove warning C4996,

#include <arpa/inet.h>


namespace base
{
void setAddr(const char * ip, const uint16_t port, struct sockaddr_in * pAddr);
struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
void setReuseAddr(int sockfd, bool on);
void setReusePort(int sockfd, bool on);
void setTcpNoDely(int sockfd, bool on);
void setKeepAlive(int sockfd, bool on, int keepIdle = 60, int keepInterval = 5, int keepCount = 3);
}




#endif // _SOCKET_OPT_H_
