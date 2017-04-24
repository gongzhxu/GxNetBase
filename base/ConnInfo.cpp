#include "ConnInfo.h"


void ConnInfo::addAddrInfo(const AddrInfo & info)
{
    _addrinfo.emplace_back(info);
}

void ConnInfo::addAddrInfo(int sa_family, std::string ip, uint32_t port)
{
    _addrinfo.emplace_back(AddrInfo(sa_family, ip, port));
}

const AddrInfo & ConnInfo::getNextAddrInfo()
{
    if(_addrinfo.size() == 0)
    {
        static AddrInfo addrInfo;
        return addrInfo;
    }

    if(_next >= _addrinfo.size())
    {
        _next = 0;
    }

    return _addrinfo[_next++];
}

const AddrInfo & ConnInfo::getCurrAddrInfo()
{
    if(_addrinfo.size() == 0)
    {
        static AddrInfo addrInfo;
        return addrInfo;
    }

    size_t curr = 0;
    if(_next > 0)
    {
        curr = _next-1;
        if(curr >= _addrinfo.size())
        {
            curr = 0;
        }
    }

    return _addrinfo[curr];
}
