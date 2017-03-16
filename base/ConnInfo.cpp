#include "ConnInfo.h"


void ConnInfo::addAddrInfo(const AddrInfo & info)
{
    _addrinfo.emplace_back(info);
}

void ConnInfo::addAddrInfo(int sa_family, std::string ip, uint32_t port)
{
    _addrinfo.emplace_back(AddrInfo(sa_family, ip, port));
}
