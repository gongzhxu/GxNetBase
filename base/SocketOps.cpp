#include "SocketOps.h"

#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "BaseUtil.h"

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

int base::makeAddr(const AddrInfo & addrInfo, sockaddr_storage & storage)
{
    if(addrInfo.sa_family()  == AF_INET)
    {
        sockaddr_in & addr = *(sockaddr_in *)&storage;
        addr.sin_family = addrInfo.sa_family();
        addr.sin_port = htons(addrInfo.port());
        addr.sin_addr.s_addr = inet_addr(addrInfo.ip().c_str());
        return sizeof(sockaddr_in);
    }
    else if(addrInfo.sa_family() == AF_INET6)
    {
        sockaddr_in6 & addr6 = *(sockaddr_in6 *)&storage;
        addr6.sin6_family = addrInfo.sa_family();
        addr6.sin6_port = htons(addrInfo.port());
        inet_pton(AF_INET6, addrInfo.ip().c_str(), (void *)&(addr6.sin6_addr));
        return sizeof(sockaddr_in6);
    }

    return 0;
}

AddrInfo base::getAddr(struct sockaddr * sockAddr, int sockLen)
{
    if(sockLen == sizeof(sockaddr_in))
    {
        sockaddr_in & addr = *(sockaddr_in *)sockAddr;
        return AddrInfo(AF_INET, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
    else if(sockLen == sizeof(sockaddr_in6))
    {
        sockaddr_in6 & addr6 = *(sockaddr_in6 *)sockAddr;
        char szIp[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, &(addr6.sin6_addr), szIp, sizeof(szIp));
        return AddrInfo(AF_INET6, szIp, ntohs(addr6.sin6_port));
    }

    return AddrInfo();
}

AddrInfo base::getLocalAddr(int sockfd)
{
    sockaddr_storage storage;
    socklen_t sockLen = sizeof(storage);
    if(::getpeername(sockfd, (struct sockaddr*)&storage, &sockLen) < 0)
    {
        //LOG
    }

    if(sockLen == sizeof(sockaddr_in))
    {
        sockaddr_in & addr = *(sockaddr_in *)&storage;
        return AddrInfo(AF_INET, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
    else if(sockLen == sizeof(sockaddr_in6))
    {
        sockaddr_in6 & addr6 = *(sockaddr_in6 *)&storage;
        char szIp[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, &(addr6.sin6_addr), szIp, sizeof(szIp));
        return AddrInfo(AF_INET6, szIp, ntohs(addr6.sin6_port));
    }

    return AddrInfo();
}

AddrInfo base::getPeerAddr(int sockfd)
{
    sockaddr_storage storage;
    socklen_t sockLen = sizeof(storage);
    if(::getpeername(sockfd, (struct sockaddr*)&storage, &sockLen) < 0)
    {
        //LOG
    }

    if(sockLen == sizeof(sockaddr_in))
    {
         sockaddr_in & addr = *(sockaddr_in *)&storage;
         return AddrInfo(AF_INET, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
    else if(sockLen == sizeof(sockaddr_in6))
    {
        sockaddr_in6 & addr6 = *(sockaddr_in6 *)&storage;
        char szIp[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, &(addr6.sin6_addr), szIp, sizeof(szIp));
        return AddrInfo(AF_INET6, szIp, ntohs(addr6.sin6_port));
    }

    return AddrInfo();
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
    ::setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
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
