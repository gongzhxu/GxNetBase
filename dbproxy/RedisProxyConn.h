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

    bool command(const char * format, ...);
    bool vcommand(const char * format, ...);

    bool exists(const char * key);
    std::string get(const char * key);
    bool mget(const KeyList & keys, ValueMap & retValue);
    bool hexists(const char * key, const char * item);
    bool hdel(const char * key, const ItemList & items);
    bool hmset(const char * key, const ItemList & items, const char * value);
    bool hmset(const char * key, const ValueMap & valueMap);
    std::string hget(const char * key, const char * item);
    bool hmget(const char * key, const ItemList & items, ValueMap & retValue);
    bool hgetall(const char * key, ValueMap & retValue);
    bool hgetall(const char * key, LONGValueMap & retValue);
    bool hgetall(const char * key, LONGValueList & retValue, bool bKey = true);
    bool smembers(const char * key, ValueList & retValue);
    bool smembers(const char * key, LONGValueList & retValue);
    bool sismember(const char * key, long item);
    long scard(const char * key);
    long incr(const char * key);
    long incrby(const char * key, long value);
    long hincrby(const char * key, const char * item, long value);
    bool expire_day(const char * key, int days);
    bool persist(const char * key);
private:
    redisReply * _vcommand(const char * format, ...);

private:
    std::string           addrs_;
    redisClusterContext *  context_;
};

#endif //_REDIS_PROXY_CONN_H_
