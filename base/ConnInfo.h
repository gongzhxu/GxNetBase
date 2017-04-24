#ifndef _SERVER_INFO_H_
#define _SERVER_INFO_H_

#include <vector>
#include <string>
#include <functional>
#include <arpa/inet.h>

class AddrInfo
{
public:
    AddrInfo():
        _sa_family(AF_INET), _ip(""), _port(0)
    {}

    AddrInfo(int sa_family, std::string ip, uint32_t port):
        _sa_family(sa_family), _ip(ip), _port(port)
    {}

    AddrInfo(const AddrInfo & ai):
        _sa_family(ai._sa_family), _ip(ai._ip), _port(ai._port)
    {}

    AddrInfo & operator=(const AddrInfo & ai)
    {
        if(this != &ai)
        {
            _sa_family = ai._sa_family;
            _ip = ai._ip;
            _port = ai._port;
        }
        return *this;
    }

    int sa_family() const { return _sa_family; }
    const std::string & ip() const { return _ip;}
    uint32_t port() const { return _port; }
private:
    int _sa_family; // the address family type(IPV4 or IPV6)
    std::string _ip; // the ip address
    uint32_t _port; // the net port
};

class ConnInfo:public std::less_equal<ConnInfo>
{
public:
    ConnInfo(int fd = -1):
        _type(0), _id(0), _hostname(""), _fd(fd), _retry(1), _next(0)
    {}

    ConnInfo(int type, uint32_t id, std::string hostname, int fd = -1, int retry = 1):
        _type(type), _id(id), _hostname(hostname), _fd(fd), _retry(retry), _next(0)
    {}

    ConnInfo(const ConnInfo & ci):
       _type(ci._type), _id(ci._id), _hostname(ci._hostname), _fd(ci._fd), _retry(ci._retry), _next(0)
    {
        _addrinfo.insert(_addrinfo.end(), ci._addrinfo.begin(), ci._addrinfo.end());
    }

    ConnInfo & operator=(const ConnInfo & ci)
    {
        if(this != &ci)
        {
            _id = ci._id;
            _hostname = ci._hostname;
            _type = ci._type;
            _fd = ci._fd;
            _retry = ci._retry;
            _next = ci._next;
            _addrinfo.insert(_addrinfo.end(), ci._addrinfo.begin(), ci._addrinfo.end());
        }
        return *this;
    }

    int type() const { return _type; }
    int id() const { return _id; }
    const std::string & hostname() const { return _hostname; }
    int fd() const { return _fd; }
    int retry() const { return _retry; }
    const std::vector<AddrInfo> & addrinfo() const { return _addrinfo; }
    const AddrInfo & addrinfo(size_t i) const { return _addrinfo[i]; }

    void setFd(const int fd) { _fd = fd; }
    void addAddrInfo(const AddrInfo & info);
    void addAddrInfo(int sa_family, std::string ip, uint32_t port);

    const AddrInfo & getNextAddrInfo();
    const AddrInfo & getCurrAddrInfo();

private:
    int                      _type;
    int                      _id;
    std::string             _hostname;

    int                      _fd;
    int                      _retry;
    size_t                  _next;
    std::vector<AddrInfo> _addrinfo;
};

inline bool operator<(const ConnInfo & lhs, const ConnInfo & rhs)
{
    if(lhs.type() < rhs.type())
    {
        return true;
    }
    else if(lhs.type() == rhs.type())
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
    }

    return false;
}

inline bool operator==(const ConnInfo & lhs, const ConnInfo & rhs)
{
    return lhs.type() == rhs.type() && lhs.id() == rhs.id() && lhs.hostname() == rhs.hostname();
}


#endif
