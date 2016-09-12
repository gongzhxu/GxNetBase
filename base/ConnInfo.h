#ifndef _SERVER_INFO_H_
#define _SERVER_INFO_H_

#include <vector>
#include <string>
#include <functional>
#include <arpa/inet.h>

class AddrInfo
{
public:
    AddrInfo(int sa_family = AF_INET, std::string ip = "", uint32_t port  = 0):
        _sa_family(sa_family), _ip(ip), _port(port)
    {}

    AddrInfo(const AddrInfo & ai):
        _sa_family(ai._sa_family), _ip(ai._ip), _port(ai._port)
    {}

    AddrInfo & operator=(const AddrInfo & ai)
    {
        _sa_family = ai._sa_family;
         _ip = ai._ip;
         _port = ai._port;
        return *this;
    }

    int sa_family() const { return _sa_family; }
    const std::string & ip() const { return _ip;}
    uint32_t port() const { return _port; }
private:
    int _sa_family;
    std::string _ip;
    uint32_t _port;
};

class ConnInfo:public std::less_equal<ConnInfo>
{
public:
    ConnInfo(int id = 0, std::string hostname = "", int retry = 1):
        _id(id), _hostname(hostname), _retry(retry), _next(0)
    {}

    ConnInfo(int id, std::string hostname, std::vector<AddrInfo> & addrinfo, int retry = 1):
        _id(id), _hostname(hostname), _retry(retry), _next(0)
    {
        _addrinfo.insert(_addrinfo.end(), addrinfo.begin(), addrinfo.end());
    }

    ConnInfo(const ConnInfo & ci):
        _id(ci._id), _hostname(ci._hostname), _retry(ci._retry), _next(0)
    {
        _addrinfo.insert(_addrinfo.end(), ci._addrinfo.begin(), ci._addrinfo.end());
    }

    ConnInfo & operator=(const ConnInfo & ci)
    {
        _id = ci._id;
        _hostname = ci._hostname;
        _retry = ci._retry;
        _next = ci._next;
        _addrinfo.insert(_addrinfo.end(), ci._addrinfo.begin(), ci._addrinfo.end());
        return *this;
    }



    int id() const { return _id; }
    const std::string & hostname() const { return _hostname; }
    const std::vector<AddrInfo> & addrinfo() const { return _addrinfo; }
    const AddrInfo & addrinfo(size_t i) const { return _addrinfo[i]; }
    int retry() const { return _retry; }

    void addAddrInfo(const AddrInfo & info);
    void addAddrInfo(int sa_family, std::string ip, uint32_t port);

    AddrInfo getNextAddrInfo()
    {
        if(_addrinfo.size() == 0)
        {
            return AddrInfo();
        }

        if(_next >= _addrinfo.size())
        {
            _next = 0;
        }

        return _addrinfo[_next++];
    }

    AddrInfo getCurrAddrInfo()
    {
        if(_addrinfo.size() == 0)
        {
            return AddrInfo();
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
private:
    int         _id;
    std::string _hostname;
    std::vector<AddrInfo> _addrinfo;
    int         _retry;
    size_t      _next;
};

inline bool operator<(const ConnInfo & lhs, const ConnInfo & rhs)
{
    if(lhs.id() < rhs.id())
    {
        return true;
    }
    else if(lhs.id() == rhs.id())
    {
        if(lhs.hostname() < rhs.hostname())
        {
            return true;
        }
    }

    return false;
}

inline bool operator==(const ConnInfo & lhs, const ConnInfo & rhs)
{
    return lhs.id() == rhs.id() && lhs.hostname() == rhs.hostname();
}


#endif
