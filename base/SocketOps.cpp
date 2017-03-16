#include "SocketOps.h"

#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "BaseUtil.h"

int base::setAddr(int sa_family, const char * ip, const uint16_t port, base::SockAddr * pAddr)
{
    memset(pAddr, 0, sizeof(base::SockAddr));

    if(sa_family == AF_INET)
    {
        pAddr->addr.sin_family = sa_family;
        pAddr->addr.sin_port = htons(port);
        pAddr->addr.sin_addr.s_addr = inet_addr(ip);
        return sizeof(pAddr->addr);
    }
    else if(sa_family == AF_INET6)
    {
        pAddr->addr6.sin6_family = sa_family;
        pAddr->addr6.sin6_port = htons(port);
        inet_pton(AF_INET6, ip, (void *)&(pAddr->addr6.sin6_addr));
        return sizeof(pAddr->addr6);
    }

    return 0;
}

void base::getLocalAddr(int sockfd, std::string & ip, uint16_t & port)
{
    base::SockAddr localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t addrLen = sizeof(localAddr);
    if(::getpeername(sockfd, (struct sockaddr*)&localAddr, &addrLen) < 0)
    {
        //LOG
    }

    if(addrLen == sizeof(localAddr.addr))
    {
        ip = inet_ntoa(localAddr.addr.sin_addr);
        port = ntohs(localAddr.addr.sin_port);
    }
    else if(addrLen == sizeof(localAddr.addr6))
    {
        char szIp[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, &(localAddr.addr6.sin6_addr), szIp, sizeof(szIp));

        ip = szIp;
        port = ntohs(localAddr.addr6.sin6_port);
    }
}

void base::getPeerAddr(int sockfd, std::string & ip, uint16_t & port)
{
    base::SockAddr peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t addrLen = sizeof(peerAddr);
    if(::getpeername(sockfd, (struct sockaddr*)&peerAddr, &addrLen) < 0)
    {
        //LOG
    }

    if(addrLen == sizeof(peerAddr.addr))
    {
        ip = inet_ntoa(peerAddr.addr.sin_addr);
        port = ntohs(peerAddr.addr.sin_port);
    }
    else if(addrLen == sizeof(peerAddr.addr6))
    {
        char szIp[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, &(peerAddr.addr6.sin6_addr), szIp, sizeof(szIp));

        ip = szIp;
        port = ntohs(peerAddr.addr6.sin6_port);
    }
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

bool base::isZeroAddr(int sa_family, std::string & ip)
{
    if(sa_family == AF_INET)
    {
        struct in_addr addr;
        inet_pton(sa_family, ip.c_str(), &addr);
        return addr.s_addr == INADDR_ANY;
    }
    else if(sa_family == AF_INET6)
    {
        struct in6_addr addr;
        inet_pton(sa_family, ip.c_str(), &addr);
        for(int i = 0; i < 4; ++i)
        {
            if(addr.s6_addr32[i] != in6addr_any.s6_addr32[i])
            {
               return false;
            }
        }

        return true;
    }

    return false;
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

    char szMac[16] = {0};

    while(ifaddrsVar!=NULL)
    {
        memset(szMac, 0, sizeof(szMac));
        struct ifreq ifreqVar;
        strncpy(ifreqVar.ifr_name, ifaddrsVar->ifa_name, sizeof(ifreqVar.ifr_name));

        if(ioctl(sockfd, SIOCGIFHWADDR, &ifreqVar) == 0)
        {
            snprintf(szMac, sizeof(szMac), "%02x%02x%02x%02x%02x%02x",
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[0],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[1],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[2],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[3],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[4],
                        (unsigned char)ifreqVar.ifr_hwaddr.sa_data[5]);
            if(strcmp(szMac, "000000000000") != 0)
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

void base::getAddrInfo(std::vector<AddrInfo> & addrInfos, uint32_t port, bool bIpv6)
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
            addrInfos.emplace_back(AddrInfo(AF_INET, szIp, port));
        }
        else if(ifaddrsVar->ifa_addr->sa_family==AF_INET6)
        {
            if(bIpv6)
            {
                void * tmpAddrPtr=&((struct sockaddr_in6 *)ifaddrsVar->ifa_addr)->sin6_addr;
                char szIp[INET6_ADDRSTRLEN] = {0};
                inet_ntop(AF_INET6, tmpAddrPtr, szIp, sizeof(szIp));
                addrInfos.emplace_back(AddrInfo(AF_INET6, szIp, port));
            }
        }

        ifaddrsVar=ifaddrsVar->ifa_next;
    }
}
