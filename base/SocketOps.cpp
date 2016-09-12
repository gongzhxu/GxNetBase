#include "SocketOps.h"

#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "BaseUtil.h"

void base::setAddr(int sa_family, const char * ip, const uint16_t port, struct sockaddr_in * pAddr)
{
    memset(pAddr, 0, sizeof(struct sockaddr_in));
    pAddr->sin_family = sa_family;
    pAddr->sin_port = htons(port);
    pAddr->sin_addr.s_addr = inet_addr(ip);
}

struct sockaddr_in base::getLocalAddr(int sockfd)
{
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t addrLen = sizeof(localAddr);
    if(::getpeername(sockfd, (struct sockaddr*)&localAddr, &addrLen) < 0)
    {
        //LOG
    }
    return localAddr;
}

struct sockaddr_in base::getPeerAddr(int sockfd)
{
    struct sockaddr_in peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t addrLen = sizeof(peerAddr);
    if(::getpeername(sockfd, (struct sockaddr*)&peerAddr, &addrLen) < 0)
    {
        //LOG
    }
    return peerAddr;
}

void base::setReuseAddr(int sockfd, bool on)
{
    int optval = on? 1: 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void base::setReusePort(int sockfd, bool on)
{
    int optval = on? 1: 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void base::setTcpNoDely(int sockfd, bool on)
{
    int optval = on? 1: 0;
    ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void base::setKeepAlive(int sockfd, bool on,  int keepIdle, int keepInterval, int keepCount)
{
    int optval = on? 1: 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
    if(on)
    {
        ::setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &keepIdle, static_cast<socklen_t>(sizeof(keepIdle)));
        ::setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &keepInterval, static_cast<socklen_t>(sizeof(keepInterval)));
        ::setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &keepCount, static_cast<socklen_t>(sizeof(keepCount)));
    }
}

bool base::isZeroAddr(int sa_family, const char * ip)
{
    struct in_addr addr;
    inet_pton(sa_family, ip, &addr);
    return addr.s_addr == 0;
}


std::string base::getHostMac()
{
    struct ifaddrs * ifaddrsVar = nullptr;
    getifaddrs(&ifaddrsVar);
    if(ifaddrsVar == nullptr)
    {
        return "";
    }

    int sockfd;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        return "";
    }

    char szMac[64] = {0};

    while(ifaddrsVar!=NULL)
    {
        memset(szMac, 0, sizeof(szMac));
        struct ifreq ifreqVar;
        strncpy(ifreqVar.ifr_name, ifaddrsVar->ifa_name, sizeof(ifreqVar.ifr_name));

        if(ioctl(sockfd, SIOCGIFHWADDR, &ifreqVar) == 0)
        {
            snprintf(szMac, sizeof(szMac), "%02x:%02x:%02x:%02x:%02x:%02x",
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[0],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[1],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[2],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[3],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[4],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[5]);
            if(strcmp(szMac, "00:00:00:00:00:00") != 0)
            {
                break;
            }
        }

        ifaddrsVar=ifaddrsVar->ifa_next;
    }
    close(sockfd);
    return szMac;
}

std::string base::getHostName()
{
    char hostname[1024] = {0};
    gethostname(hostname, sizeof(hostname));
    return hostname;
}

void base::getAddrInfo(std::vector<AddrInfo> & addrInfos, uint32_t port)
{
    struct ifaddrs * ifaddrsVar = nullptr;
    getifaddrs(&ifaddrsVar);

    while(ifaddrsVar!=NULL)
    {
        if(ifaddrsVar->ifa_addr->sa_family==AF_INET)
        {
            void * tmpAddrPtr=&((struct sockaddr_in *)ifaddrsVar->ifa_addr)->sin_addr;
            char szIp[INET_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET, tmpAddrPtr, szIp, sizeof(szIp));
            addrInfos.push_back(AddrInfo(AF_INET, szIp, port));
        }
        else if(ifaddrsVar->ifa_addr->sa_family==AF_INET6)
        {
            void * tmpAddrPtr=&((struct sockaddr_in *)ifaddrsVar->ifa_addr)->sin_addr;
            char szIp[INET6_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET6, tmpAddrPtr, szIp, sizeof(szIp));
            addrInfos.push_back(AddrInfo(AF_INET6, szIp, port));
        }

        ifaddrsVar=ifaddrsVar->ifa_next;
    }
}
