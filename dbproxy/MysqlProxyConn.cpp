#include "MysqlProxyConn.h"

#include <mysql/errmsg.h>
#include "base/BaseUtil.h"

#define	DBRESULT_SUCCESS	    0
#define	DBRESULT_UNEXCEPTION	-1

MysqlProxyConn::MysqlProxyConn(const MysqlConnInfo & info):
    _info(info),
    _bConnect(false),
    _num(0)
{
    init();
}

MysqlProxyConn::~MysqlProxyConn()
{
    release();
}

bool MysqlProxyConn::init()
{
    if(!mysql_init(&_mysql))
    {
        LOG_WARN("connect mysql error");
        return false;
    }

    unsigned int nTimeout = 60;
	mysql_options(&_mysql,MYSQL_OPT_CONNECT_TIMEOUT, &nTimeout);
	mysql_options(&_mysql,MYSQL_OPT_READ_TIMEOUT, &nTimeout);
	mysql_options(&_mysql,MYSQL_OPT_WRITE_TIMEOUT, &nTimeout);
	my_bool bReconnect = 1;
	mysql_options(&_mysql,MYSQL_OPT_RECONNECT, &bReconnect);

    if(!mysql_real_connect(&_mysql, _info.host.c_str(), _info.user.c_str(), _info.passwd.c_str(), _info.database.c_str(), _info.port, nullptr, 0))
    {
        mysql_close(&_mysql);
        LOG_WARN("connect mysql error");
        return false;
    }

    if(mysql_set_character_set(&_mysql, "utf8") != 0)
    {
        mysql_close(&_mysql);
        LOG_WARN("connect mysql error");
        return false;
    }

	_bConnect = true;
	LOG_INFO("connect mysql successed");
	return true;
}

void MysqlProxyConn::release()
{
    if(_bConnect)
    {
        mysql_close(&_mysql);
        _bConnect = false;
    }
}

bool MysqlProxyConn::command(const char * szCmd, int nLength)
{
#if 1
    LOG_DEBUG("mysql cmd:%s, %d", szCmd, nLength);
#else
     LOG_INFO("mysql cmd:%s, %d", szCmd, nLength);
#endif

    int nRet = mysql_real_query(&_mysql, szCmd, nLength);
	if(nRet != DBRESULT_SUCCESS)
    {
        unsigned int nErr = mysql_errno(&_mysql);
        LOG_WARN("mysql query error:%d, %d", nRet, nErr);
        if(nErr == CR_SERVER_GONE_ERROR ||
			nErr == CR_SERVER_LOST ||
			nErr == CR_CONN_HOST_ERROR)
        {

			LOG_INFO("mysql connection disconnect");
			release();
			if(init())
            {
                nRet = mysql_real_query(&_mysql, szCmd, nLength);
            }
		}
    }

    _num++;
    return nRet == DBRESULT_SUCCESS? true: false;
}

bool MysqlProxyConn::autocommit(bool on)
{
    if(on)
    {
        if(mysql_autocommit(&_mysql, 1) != 0)
        {
            release();
            return false;
        }
    }
    else
    {
        if(mysql_autocommit(&_mysql, 0) != 0)
        {
            release();
            return false;
        }
    }

    return true;
}

bool MysqlProxyConn::commit()
{
    _num = 0;
    return mysql_commit(&_mysql);
}

