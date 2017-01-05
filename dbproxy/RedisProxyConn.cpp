#include "RedisProxyConn.h"

#include "base/BaseUtil.h"

#define REDIS_CONNECT_TIMEOUT 200000

RedisProxyConn::RedisProxyConn(const char * addrs):
    _addrs(addrs),
    _pContext(nullptr)
{
    init();
}

RedisProxyConn::~RedisProxyConn()
{
    release();
}

bool RedisProxyConn::init()
{
    if(_pContext != nullptr)
    {
        return true;
    }

    struct timeval timeout = {0, REDIS_CONNECT_TIMEOUT};
	_pContext = redisClusterConnectWithTimeout(_addrs.c_str(), timeout, HIRCLUSTER_FLAG_NULL);
    if(!_pContext || _pContext->err)
    {
		if(_pContext)
		{
			release();
		}
		else
		{
			LOG_INFO("redisConnect failed");
		}

		return false;
    }

    LOG_INFO("connect redis success");
    return true;
}

void RedisProxyConn::release()
{
    if(_pContext)
    {
        LOG_INFO("redisCommand failed:%s", _pContext->errstr);
        redisClusterFree(_pContext);
        _pContext = nullptr;
    }
}

bool RedisProxyConn::command(const char *format, ...)
{
	if(!init())
    {
        return false;
    }


#if 1
    {
        va_list arglist;
        va_start(arglist, format);
        char szCmd[1024] = {0};
        vsnprintf(szCmd, 1024, format, arglist);
        LOG_INFO("szCmd:%s", szCmd);
        va_end(arglist);
    }
#endif // 1

    va_list arglist;
    va_start(arglist, format);

    bool bValue = false;
	redisReply * reply = (redisReply *)redisClustervCommand(_pContext, format, arglist);
	if(!reply)
    {
        release();
    }
    else
    {
        freeReplyObject(reply);
        bValue = true;
    }
    va_end(arglist);
	return bValue;
}


redisReply * RedisProxyConn::commandv(const char * format, ...)
{
    if(!init())
    {
        return nullptr;
    }


#if 1
    {
        va_list arglist;
        va_start(arglist, format);
        char szCmd[1024] = {0};
        vsnprintf(szCmd, 1024, format, arglist);
        LOG_INFO("szCmd:%s", szCmd);
        va_end(arglist);
    }
#endif // 1

    va_list arglist;
    va_start(arglist, format);
	redisReply * reply = (redisReply *)redisClustervCommand(_pContext, format, arglist);
    va_end(arglist);

	return reply;
}

bool RedisProxyConn::exists(const char * key)
{
    if(!init())
    {
        return false;
    }

    redisReply * reply = (redisReply*) redisClusterCommand(_pContext, "EXISTS %s", key);
    if(!reply)
    {
        release();
        return false;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);

    return 0 == ret_value? false: true;
}

std::string RedisProxyConn::get(const char * key)
{
    std::string ret_value;

    redisReply * reply = commandv("GET %s", key);
	if(!reply)
    {
		release();
		return ret_value;
	}

	if(reply->type == REDIS_REPLY_STRING)
    {
		ret_value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return ret_value;
}

bool RedisProxyConn::mget(const KeyList & keys, ValueMap & retValue)
{
    assert(!keys.empty());
    if(!init())
    {
        return false;
    }

    std::string strCmd = "MGET";
    for(KeyList::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        strCmd += " " + *it;
    }

    redisReply * reply = (redisReply*) redisClusterCommand(_pContext, strCmd.c_str());
    if(!reply)
    {
        release();
        return false;
    }

    assert(reply->type == REDIS_REPLY_ARRAY);

    for(size_t i=0; i<reply->elements; ++i)
    {
        redisReply * child_reply = reply->element[i];
        if(child_reply->type == REDIS_REPLY_STRING)
        {
            retValue[keys[i]] = child_reply->str;
        }
    }

    freeReplyObject(reply);
    return true;
}

bool RedisProxyConn::hexists(const char * key, const char * item)
{
    redisReply * reply = commandv("HEXISTS %s %s", key, item);
    if(!reply)
    {
        release();
        return false;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);

    return 0 == ret_value? false: true;
}

bool RedisProxyConn::hdel(const char * key, const ItemList & iterms)
{
    if(!init())
    {
        return false;
    }

    int argc = iterms.size() + 2;
    const char ** argv = new const char *[argc];
    if(!argv)
    {
        return false;
    }

    argv[0] = "HDEL";
    argv[1] = key;
    int index = 2;
    for(size_t i = 0; i < iterms.size(); ++i)
    {
        argv[index++] = iterms[i].c_str();
    }

	redisReply* reply = (redisReply *)redisClusterCommandArgv(_pContext, argc, argv, NULL);
	if (!reply)
    {
		delete []argv;
		release();
        return false;
	}

	delete []argv;

	long ret_value = reply->integer;
    freeReplyObject(reply);

    return 0 == ret_value? false: true;
}

bool RedisProxyConn::hmset(const char * key, const ItemList & iterms, const char * value)
{
    if(!init())
    {
        return false;
    }

    int argc = iterms.size()*2 + 2;
    const char ** argv = new const char *[argc];
    if(!argv)
    {
        return false;
    }

    argv[0] = "HMSET";
    argv[1] = key;
    int index = 2;
    for(size_t i = 0; i < iterms.size(); ++i)
    {
        argv[index++] = iterms[i].c_str();
        argv[index++] = value;
    }

	redisReply* reply = (redisReply *)redisClusterCommandArgv(_pContext, argc, argv, NULL);
	if (!reply)
    {
		delete []argv;
		release();
        return false;
	}

	delete []argv;

	long ret_value = reply->integer;
    freeReplyObject(reply);

    return 0 == ret_value? false: true;
}

std::string RedisProxyConn::hget(const char * key, const char * iterm)
{
    std::string ret_value;

    redisReply * reply = commandv("HGET %s %s", key, iterm);
	if(!reply)
    {
		release();
		return ret_value;
	}

	if(reply->type == REDIS_REPLY_STRING)
    {
		ret_value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return ret_value;
}

bool RedisProxyConn::hmget(const char * key, const ItemList & items, ValueMap & retValue)
{
    assert(key && !items.empty());

    if(!init())
    {
        return false;
    }

    std::string strCmd = "HMGET ";
    strCmd += key;
    for(ItemList::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        strCmd += " " + *it;
    }

    LOG_DEBUG("szCmd:%s", strCmd.c_str());
    redisReply * reply = (redisReply*) redisClusterCommand(_pContext, strCmd.c_str());
    if(!reply)
    {
        release();
        return false;
    }

    //assert(reply->type == REDIS_REPLY_ARRAY);
    for(size_t i = 0; i < reply->elements; ++i)
    {
        redisReply* child_reply = reply->element[i];
        if(child_reply->type == REDIS_REPLY_STRING)
        {
            retValue[items[i]] = child_reply->str;
        }
    }

    freeReplyObject(reply);
    return true;
}

bool RedisProxyConn::hgetall(const char * key, ValueMap & retValue)
{
    bool bValue = false;
	redisReply * reply = commandv("HGETALL %s", key);
	if(!reply)
	{
		release();
	}
    else
    {
        if((reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0))
        {
            for(size_t i = 0; i < reply->elements; i += 2)
            {
                redisReply * field_reply = reply->element[i];
                redisReply * value_reply = reply->element[i + 1];

                std::string field(field_reply->str, field_reply->len);
                std::string value(value_reply->str, value_reply->len);
                retValue.insert(make_pair(field, value));
            }
        }

        freeReplyObject(reply);
        bValue = true;
    }

	return bValue;
}

bool RedisProxyConn::hgetall(const char * key, LONGValueMap & retValue)
{
    bool bValue = false;
	redisReply * reply = commandv("HGETALL %s", key);
	if(!reply)
	{
		release();
	}
    else
    {
        if((reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0))
        {
            for(size_t i = 0; i < reply->elements; i += 2)
            {
                redisReply * field_reply = reply->element[i];
                redisReply * value_reply = reply->element[i + 1];

                retValue.insert(std::make_pair(atoll(field_reply->str), atoll(value_reply->str)));
            }
        }

        freeReplyObject(reply);
        bValue = true;
    }

	return bValue;
}

bool RedisProxyConn::hgetall(const char * key, LONGValueList & retValue, bool bKey)
{
    bool bValue = false;
	redisReply * reply = commandv("HGETALL %s", key);
	if(!reply)
	{
		release();
	}
    else
    {
        if((reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0))
        {
            int nPos = bKey? 0: 1;
            for(size_t i = 0; i < reply->elements; i += 2)
            {
                redisReply * element_reply = reply->element[i+nPos];
                retValue.push_back(atoll(element_reply->str));
            }
        }

        freeReplyObject(reply);
        bValue = true;
    }

	return bValue;
}

bool RedisProxyConn::smembers(const char * key, ValueList & retValue)
{
    redisReply * reply = commandv("SMEMBERS %s", key);
    if(!reply)
    {
        release();
        return false;
    }

    for(size_t i = 0; i < reply->elements; ++i)
    {
        redisReply* child_reply = reply->element[i];
        if(child_reply->type == REDIS_REPLY_STRING)
        {
            retValue.push_back(child_reply->str);
        }
    }

    freeReplyObject(reply);
    return true;
}

bool RedisProxyConn::smembers(const char * key, LONGValueList & retValue)
{
    redisReply * reply = commandv("SMEMBERS %s", key);
    if(!reply)
    {
        release();
        return false;
    }

    for(size_t i = 0; i < reply->elements; ++i)
    {
        redisReply* child_reply = reply->element[i];
        if(child_reply->type == REDIS_REPLY_STRING)
        {
            retValue.push_back(atol(child_reply->str));
            //retValue.push_back(child_reply->integer);
        }
    }

    freeReplyObject(reply);
    return true;
}

bool RedisProxyConn::sismember(const char * key, long item)
{
    redisReply * reply = commandv("SISMEMBER %s %lu", key, item);
    if(!reply)
    {
        release();
        return false;
    }

    long ret_value = reply->integer;
    freeReplyObject(reply);

    return 0 == ret_value? false: true;
}

long RedisProxyConn::incr(const char * key)
{
    redisReply * reply = commandv("INCR %s", key);
    if(!reply)
    {
        release();
        return -1;
    }

    long ret_value = reply->integer;

    freeReplyObject(reply);
    return ret_value;
}

long RedisProxyConn::incrby(const char * key, long value)
{
    redisReply * reply = commandv("INCRBY %s %lu", key, value);
    if(!reply)
    {
        release();
        return -1;
    }

    long ret_value = reply->integer;

    freeReplyObject(reply);
    return ret_value;
}

bool RedisProxyConn::expire_day(const char * key, int days)
{
    redisReply * reply = commandv("EXPIRE %s %lu", key, 86400*days);
    if(!reply)
    {
        release();
        return false;
    }

    long ret_value = reply->integer;

    freeReplyObject(reply);
    return ret_value;
}

bool RedisProxyConn::persist(const char * key)
{
    redisReply * reply = commandv("PERSIST %s", key);
    if(!reply)
    {
        release();
        return false;
    }

    long ret_value = reply->integer;

    freeReplyObject(reply);
    return ret_value;
}
