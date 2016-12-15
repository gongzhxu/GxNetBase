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
        _mysql(mysql)
    {
        _res = mysql_store_result(mysql);
    }

    ~AutoMysqlRes()
    {
        mysql_free_result(_res);
        while(!mysql_next_result(_mysql))
        {
            _res = mysql_store_result(_mysql);
            mysql_free_result(_res);
        }
    }

    MYSQL_RES * res() { return _res; }
private:
    MYSQL * _mysql;
    MYSQL_RES * _res;
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

    MYSQL * mysql() { return &_mysql; }
    int num() { return _num; }


    bool command(const char * szCmd, int nLength);
    bool commit();
private:
    MysqlConnInfo _info;

    MYSQL       _mysql;
    bool        _bConnect;
    int         _num;
};

#endif //_MYSQL_PROXY_CONN_H_
