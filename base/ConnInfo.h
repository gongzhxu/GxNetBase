#ifndef _SERVER_INFO_H_
#define _SERVER_INFO_H_

#include <vector>
#include <string>
#include <functional>

class ConnInfo:public std::less_equal<ConnInfo>
{
public:
    ConnInfo(std::string host = "", uint16_t port = 0, int id = 0, int retry = 1):
        _host(host), _port(port), _id(id), _retry(retry)
    {}

    ConnInfo(const ConnInfo & ci):
        _host(ci._host), _port(ci._port), _id(ci._id), _retry(ci._retry)
    {}

    ConnInfo & operator=(const ConnInfo & ci)
    {
        _host = ci._host;
        _port = ci._port;
        _id = ci._id;
        _retry = ci._retry;
        return *this;
    }

    std::string host() const { return _host; }
    uint16_t port() const { return _port; }
    int retry() const { return _retry; }
    int id() const { return _id; }

public:
    std::string _host;
    uint16_t    _port;
    int         _id;
    int         _retry;
};

inline bool operator<(const ConnInfo & lhs, const ConnInfo & rhs)
{
    if(lhs.host() < rhs.host())
    {
        return true;
    }
    else if(lhs.host() == rhs.host())
    {
        if(lhs.port() < rhs.port())
        {
            return true;
        }
        else if(lhs.port() == rhs.port())
        {
            if(lhs.id() < rhs.id())
            {
                return true;
            }
        }
    }

    return false;
}

inline bool operator==(const ConnInfo & lhs, const ConnInfo & rhs)
{
    return (lhs.host() == rhs.host()) && (lhs.port() == rhs.port()) && (lhs.id() == rhs.id());
}


#endif
