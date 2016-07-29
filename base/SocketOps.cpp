#include "SocketOps.h"

#include <netinet/tcp.h>

#include "BaseUtil.h"

void base::setAddr(const char * ip, const uint16_t port, struct sockaddr_in * pAddr)
{
    memset(pAddr, 0, sizeof(struct sockaddr_in));
    pAddr->sin_family = AF_INET;
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
