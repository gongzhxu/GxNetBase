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

    ConnList():next_(0) {};
    ~ConnList() {};

    BaseConnPtr getNextConn();


    void addConn(const BaseConnPtr & pConn);
    void delConn(const BaseConnPtr & pConn);

    size_t size() { return connList_.size(); }
private:
    std::mutex mutex_;
    ConnList_t connList_;
    size_t next_;
};

#endif
