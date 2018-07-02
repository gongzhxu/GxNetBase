#ifndef _MYSQL_PROXY_CONN_H_
#define _MYSQL_PROXY_CONN_H_

#include <string>

struct MysqlConnInfo
{
    std::string  host;
    std::string  user;
    std::string  passwd;
    std::string  database;
    unsigned int port;
};

typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;
typedef char **MYSQL_ROW;

class AutoMysqlRes
{
public:
    AutoMysqlRes(MYSQL * mysql);
    ~AutoMysqlRes();

    MYSQL_RES * res();
    int fields();
    bool next();

    bool fetch_row();
    bool fetch_next_row();

    const char * str_row(int i);
    int int_row(int i);
    long int long_row(int i);
    unsigned long len_row(int i);
private:
    MYSQL * mysql_;
    MYSQL_RES * res_;
    int fields_;

    MYSQL_ROW row_;
    unsigned long * len_;
};

class MysqlProxyConn
{
public:
    MysqlProxyConn(const MysqlConnInfo & info);
    virtual ~MysqlProxyConn();

public:
    bool MysqlInit();
    void release();
    bool autocommit(bool on);

    MYSQL * mysql() { return mysql_; }
    int num() { return num_; }

    std::string escape(const std::string & from);
    bool escape(const std::string & from, std::string & to);
    bool command(const char * szCmd, int nLength);
    bool query(const char * szCmd);
    bool commit();
private:
    MysqlConnInfo info_;

    MYSQL *     mysql_;
    bool         bConnect_;
    int           num_;
};

#endif //_MYSQL_PROXY_CONN_H_
