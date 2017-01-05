#ifndef _REDIS_PROXY_CONN_H_
#define _REDIS_PROXY_CONN_H_

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <hiredis-vip/hircluster.h>

typedef std::vector<std::string> KeyList;
typedef std::vector<std::string> ItemList;
typedef std::map<std::string, std::string> ValueMap;
typedef std::map<uint64_t, uint64_t> LONGValueMap;
typedef std::vector<std::string> ValueList;
typedef std::vector<uint64_t> LONGValueList;

class RedisProxyConn;
class ConfigReader;

typedef std::shared_ptr<RedisProxyConn> RedisProxyConnPtr;
#define MakeRedisProxyConnPtr std::make_shared<RedisProxyConn>

class RedisProxyConn
{
public:
    RedisProxyConn(const char * addrs);
    ~RedisProxyConn();
public:
    bool init();
    void release();

public:
    bool command(const char * format, ...);
    redisReply * commandv(const char * format, ...);

    bool exists(const char * key);
    std::string get(const char * key);
    bool mget(const KeyList & keys, ValueMap & retValue);
    bool hexists(const char * key, const char * item);
    bool hdel(const char * key, const ItemList & iterms);
    bool hmset(const char * key, const ItemList & iterms, const char * value);
    std::string hget(const char * key, const char * iterm);
    bool hmget(const char * key, const ItemList & items, ValueMap & retValue);
    bool hgetall(const char * key, ValueMap & retValue);
    bool hgetall(const char * key, LONGValueMap & retValue);
    bool hgetall(const char * key, LONGValueList & retValue, bool bKey = true);
    bool smembers(const char * key, ValueList & retValue);
    bool smembers(const char * key, LONGValueList & retValue);
    bool sismember(const char * key, long item);
    long incr(const char * key);
    long incrby(const char * key, long value);
    bool expire_day(const char * key, int days);
    bool persist(const char * key);
private:
    std::string           _addrs;
    redisClusterContext * _pContext;
};

#endif //_REDIS_PROXY_CONN_H_
