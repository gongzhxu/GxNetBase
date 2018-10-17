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
        sa_family_(AF_INET), ip_(""), port_(0)
    {}

    AddrInfo(int sa_family, std::string ip, uint32_t port):
        sa_family_(sa_family), ip_(ip), port_(port)
    {}

    AddrInfo(const AddrInfo & ai):
        sa_family_(ai.sa_family_), ip_(ai.ip_), port_(ai.port_)
    {}

    AddrInfo & operator=(const AddrInfo & ai)
    {
        if(this != &ai)
        {
            sa_family_ = ai.sa_family_;
            ip_ = ai.ip_;
            port_ = ai.port_;
        }
        return *this;
    }

    int sa_family() const { return sa_family_; }
    const std::string & ip() const { return ip_;}
    uint32_t port() const { return port_; }
private:
    int sa_family_; // the address family type(IPV4 or IPV6)
    std::string ip_; // the ip address
    uint32_t port_; // the net port
};

class ConnInfo:public std::less_equal<ConnInfo>
{
public:
    ConnInfo(int fd = -1):
        data_(0), type_(0), id_(0), hostname_(""), fd_(fd), retry_(1), next_(0)
    {}

    ConnInfo(int type, uint32_t id, std::string hostname, int fd = -1, int retry = 1):
       data_(0), type_(type), id_(id), hostname_(hostname), fd_(fd), retry_(retry), next_(0)
    {}

    ConnInfo(const ConnInfo & ci):
       data_(ci.data_), type_(ci.type_), id_(ci.id_), hostname_(ci.hostname_), fd_(ci.fd_), retry_(ci.retry_), next_(0)
    {
        addrinfo_.insert(addrinfo_.end(), ci.addrinfo_.begin(), ci.addrinfo_.end());
    }

    ConnInfo & operator=(const ConnInfo & ci)
    {
        if(this != &ci)
        {
            data_ = ci.data_;
            id_ = ci.id_;
            hostname_ = ci.hostname_;
            type_ = ci.type_;
            fd_ = ci.fd_;
            retry_ = ci.retry_;
            next_ = ci.next_;
            addrinfo_.insert(addrinfo_.end(), ci.addrinfo_.begin(), ci.addrinfo_.end());
        }
        return *this;
    }

    void * data() const { return data_; }
    int type() const { return type_; }
    int id() const { return id_; }
    const std::string & hostname() const { return hostname_; }
    int fd() const { return fd_; }
    int retry() const { return retry_; }
    const std::vector<AddrInfo> & addrinfo() const { return addrinfo_; }
    const AddrInfo & addrinfo(size_t i) const { return addrinfo_[i]; }

    void setData(void * data) { data_ = data; }
    void setFd(const int fd) { fd_ = fd; }
    void addAddrInfo(const AddrInfo & info);
    void addAddrInfo(int sa_family, std::string ip, uint32_t port);

    const AddrInfo & getNextAddrInfo();
    const AddrInfo & getCurrAddrInfo();

private:
    void *                  data_;
    int                      type_;
    int                      id_;
    std::string             hostname_;

    int                      fd_;
    int                      retry_;
    size_t                  next_;
    std::vector<AddrInfo> addrinfo_;
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
