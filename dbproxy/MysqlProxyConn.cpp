#include "MysqlProxyConn.h"

#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#include "base/BaseUtil.h"

#define	DBRESULT_SUCCESS	    0
#define	DBRESULT_UNEXCEPTION	-1

struct MysqlServiceInit
{
    MysqlServiceInit() { mysql_server_init(0, nullptr, nullptr); }
    ~MysqlServiceInit() { mysql_server_end(); }
};

AutoMysqlRes::AutoMysqlRes(MYSQL * mysql):
        mysql_(mysql),
        res_(mysql_store_result(mysql)),
        fields_(mysql_num_fields(res_))
{
}

AutoMysqlRes::~AutoMysqlRes()
{
    if(res_)
    {
        mysql_free_result(res_);
    }

    while(!mysql_next_result(mysql_))
    {
        res_ = mysql_store_result(mysql_);
        if(res_)
        {
            mysql_free_result(res_);
        }
    }
}

MYSQL_RES * AutoMysqlRes::res()
{
    return res_;
}

int AutoMysqlRes::fields()
{
    return fields_;
}

bool AutoMysqlRes::next()
{
    if(res_)
    {
        mysql_free_result(res_);
        res_ = NULL;
    }

    if(!mysql_next_result(mysql_))
    {
        res_ = mysql_store_result(mysql_);
        fields_ = mysql_num_fields(res_);
    }

    return res_ != NULL;
}

bool AutoMysqlRes::fetch_row()
{
    if(!res_ )
    {
        return false;
    }

    row_ = mysql_fetch_row(res_);
    len_ = mysql_fetch_lengths(res_);

    return row_ != NULL && len_ != NULL;
}

bool AutoMysqlRes::fetch_next_row()
{
    if(!next())
    {
        return false;
    }

    row_ = mysql_fetch_row(res_);
    len_ = mysql_fetch_lengths(res_);

    return row_ != NULL && len_ != NULL;
}

const char * AutoMysqlRes::str_row(int i)
{
    return (row_ && i < fields_)? SAFE_A2A(row_[i]): "";
}

int AutoMysqlRes::int_row(int i)
{
    return (row_ && i < fields_)? SAFE_A2I(row_[i]): 0;
}

long int AutoMysqlRes::long_row(int i)
{
    return (row_ && i < fields_)? SAFE_A2L(row_[i]): 0;
}

unsigned long AutoMysqlRes::len_row(int i)
{
    return (len_ && i < fields_)? len_[i]: 0;
}

MysqlProxyConn::MysqlProxyConn(const MysqlConnInfo & info):
    info_(info),
    bConnect_(false),
    num_(0)
{
    static MysqlServiceInit g_MysqlServiceInit;

    mysql_ = new MYSQL();
    MysqlInit();
}

MysqlProxyConn::~MysqlProxyConn()
{
    release();

    if(mysql_)
    {
        delete mysql_;
    }
}

bool MysqlProxyConn::MysqlInit()
{
    if(!mysql_init(mysql_))
    {
        LOG_INFO("init mysql error:%d,%s", mysql_errno(mysql_), mysql_error(mysql_));
        return false;
    }

    unsigned int nTimeout = 60;
	mysql_options(mysql_,MYSQL_OPT_CONNECT_TIMEOUT, &nTimeout);
	mysql_options(mysql_,MYSQL_OPT_READ_TIMEOUT, &nTimeout);
	mysql_options(mysql_,MYSQL_OPT_WRITE_TIMEOUT, &nTimeout);
	my_bool bReconnect = 1;
	mysql_options(mysql_,MYSQL_OPT_RECONNECT, &bReconnect);

    if(!mysql_real_connect(mysql_, info_.host.c_str(), info_.user.c_str(), info_.passwd.c_str(), info_.database.c_str(), info_.port, nullptr, 0))
    {
        LOG_INFO("connect mysql=%s error:%d,%s", info_.host.c_str(), mysql_errno(mysql_), mysql_error(mysql_));
        mysql_close(mysql_);
        return false;
    }

    if(mysql_set_character_set(mysql_, "utf8") != 0)
    {
        LOG_INFO("connect mysql=%s error:%d,%s", info_.host.c_str(), mysql_errno(mysql_), mysql_error(mysql_));
        mysql_close(mysql_);
        return false;
    }

	bConnect_ = true;
	LOG_INFO("connect mysql successed!!!");
	return true;
}

void MysqlProxyConn::release()
{
    if(bConnect_)
    {
        mysql_close(mysql_);
        bConnect_ = false;
    }
}

std::string MysqlProxyConn::escape(const std::string & from)
{
    std::string to;
    to.resize(from.size()*2+2);
    long len= mysql_real_escape_string(mysql_, const_cast<char *>(to.c_str()), from.c_str(), from.size());

    return len >= 0? to.substr(0, len): "";
}

bool MysqlProxyConn::escape(const std::string & from, std::string & to)
{
    if(&from == &to)
    {
        to = escape(from);
        return true;
    }
    else
    {
        to.resize(from.size()*2+2);
        long len= mysql_real_escape_string(mysql_, const_cast<char *>(to.c_str()), from.c_str(), from.size());
        if(len >= 0)
        {
            to.resize(len);
            return true;

        }
        else
        {
            to.resize(0);
            return false;
        }
    }
}

bool MysqlProxyConn::command(const char * szCmd, int nLength)
{
    LOG_DEBUG("mysql cmd:%s, %d", szCmd, nLength);

    int nRet = mysql_real_query(mysql_, szCmd, nLength);
	if(nRet != DBRESULT_SUCCESS)
    {
        unsigned int nErr = mysql_errno(mysql_);
        LOG_WARN("mysql query error:%d, %d, %s", nRet, nErr, mysql_error(mysql_));
        if(nErr == CR_SERVER_GONE_ERROR ||
			nErr == CR_SERVER_LOST ||
			nErr == CR_CONN_HOST_ERROR)
        {
			LOG_INFO("mysql connection disconnect!!!");
			release();
			if(MysqlInit())
            {
                nRet = mysql_real_query(mysql_, szCmd, nLength);
            }
		}
    }

    if(nRet != DBRESULT_SUCCESS)
    {
        LOG_WARN("mysql query error:%d, %d, %s", nRet, mysql_errno(mysql_), mysql_error(mysql_));
        return false;
    }

    return true;
}

bool MysqlProxyConn::query(const char * szCmd)
{
    int nRet = mysql_query(mysql_, szCmd);

    return nRet == DBRESULT_SUCCESS? true: false;
}

bool MysqlProxyConn::autocommit(bool on)
{
    if(on)
    {
        if(mysql_autocommit(mysql_, 1) != 0)
        {
            release();
            return false;
        }
    }
    else
    {
        if(mysql_autocommit(mysql_, 0) != 0)
        {
            release();
            return false;
        }
    }

    return true;
}

bool MysqlProxyConn::commit()
{
    num_ = 0;
    return mysql_commit(mysql_);
}
