#ifndef _SOCKET_OPT_H_
#define _SOCKET_OPT_H_

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

std::string getHostMac();
std::string getHostName();

int makeAddr(const AddrInfo & addrInfo, sockaddr_storage & storage);
AddrInfo getAddr(struct sockaddr * sockAddr, int sockLen);

AddrInfo getLocalAddr(int sockfd);
AddrInfo getPeerAddr(int sockfd);
void setReuseAddr(int sockfd, bool on);
void setReusePort(int sockfd, bool on);
void setTcpNoDely(int sockfd, bool on);
void setKeepAlive(int sockfd, bool on, int keepIdle = 60, int keepInterval = 10, int keepCount = 6);
bool isZeroAddr(int sa_family, std::string & ip);

void getAddrInfo(std::vector<AddrInfo> & addrInfos, uint32_t port = 0, bool bIpv6 = false);
}

#endif // _SOCKET_OPT_H_
