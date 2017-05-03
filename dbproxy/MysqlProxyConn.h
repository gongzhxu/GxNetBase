#ifndef _MYSQL_PROXY_CONN_H_
#define _MYSQL_PROXY_CONN_H_

#include <string>
#include <mysql/mysql.h>

#define ROW(nIndex) ((nIndex < nFields && row[nIndex])? row[nIndex]: "")
#define MYSQLLEN(nIndex) ((nIndex < nFields && len[nIndex])? len[nIndex]: 0)

struct MysqlConnInfo
{
    std::string  host;
    std::string  user;
    std::string  passwd;
    std::string  database;
    unsigned int port;
};

class AutoMysqlRes
{
public:
    AutoMysqlRes(MYSQL * mysql):
        mysql_(mysql),
        res_(mysql_store_result(mysql))
    {
    }

    MYSQL_RES * next()
    {
        if(res_)
        {
            mysql_free_result(res_);
            res_ = nullptr;
        }

        if(!mysql_next_result(mysql_))
        {
            res_ = mysql_store_result(mysql_);
        }

        return res_;
    }

    ~AutoMysqlRes()
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

    MYSQL_RES * res() { return res_; }
private:
    MYSQL * mysql_;
    MYSQL_RES * res_;
};

class MysqlProxyConn
{
public:
    MysqlProxyConn(const MysqlConnInfo & info);
    virtual ~MysqlProxyConn();

public:
    bool init();
    void release();
    bool autocommit(bool on);

    MYSQL * mysql() { return &mysql_; }
    int num() { return num_; }

    std::string escape(const std::string & from);
    bool escape(const std::string & from, std::string & to);
    bool command(const char * szCmd, int nLength);
    bool query(const char * szCmd);
    bool commit();
private:
    MysqlConnInfo info_;

    MYSQL       mysql_;
    bool         bConnect_;
    int           num_;
};

#endif //_MYSQL_PROXY_CONN_H_
