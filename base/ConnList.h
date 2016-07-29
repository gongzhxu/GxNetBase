#ifndef _CONN_LIST_H_
#define _CONN_LIST_H_

#include <vector>
#include <mutex>
#include <memory>

#include "ConnInfo.h"
class BaseConn;

typedef std::shared_ptr<BaseConn> BaseConnPtr;

class ConnList
{
public:
    typedef std::vector<BaseConnPtr> ConnList_t;

    ConnList():_next(0) {};
    ~ConnList() {};

    BaseConnPtr getNextConn();


    void addConn(const BaseConnPtr & pConn);
    void delConn(const BaseConnPtr & pConn);

    size_t size() { return _connList.size(); }
private:
    std::mutex _mutex;
    ConnList_t _connList;
    size_t _next;
};

#endif
