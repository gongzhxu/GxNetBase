#include "ConnInfo.h"


void ConnInfo::addAddrInfo(const AddrInfo & info)
{
    addrinfo_.emplace_back(info);
}

void ConnInfo::addAddrInfo(int sa_family, std::string ip, uint32_t port)
{
    addrinfo_.emplace_back(AddrInfo(sa_family, ip, port));
}

const AddrInfo & ConnInfo::getNextAddrInfo()
{
    if(addrinfo_.size() == 0)
    {
        static AddrInfo addrInfo;
        return addrInfo;
    }

    if(next_ >= addrinfo_.size())
    {
        next_ = 0;
    }

    return addrinfo_[next_++];
}

const AddrInfo & ConnInfo::getCurrAddrInfo()
{
    if(addrinfo_.size() == 0)
    {
        static AddrInfo addrInfo;
        return addrInfo;
    }

    size_t curr = 0;
    if(next_ > 0)
    {
        curr = next_-1;
        if(curr >= addrinfo_.size())
        {
            curr = 0;
        }
    }

    return addrinfo_[curr];
}
