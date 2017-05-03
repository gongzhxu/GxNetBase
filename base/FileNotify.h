#ifndef _FILE_NOTIFY_H_
#define _FILE_NOTIFY_H_

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <event2/event.h>

class EventLoop;
class FileNotify;
typedef std::shared_ptr<FileNotify> FileNotifyPtr;
#define MakeFileNotifytPtr std::make_shared<FileNotify>

namespace FileNotifyImpl
{

class FileInfo
{
public:
    typedef std::function<void(const FileInfo)> Functor;

    FileInfo():
        wd_(0)
    {}

    FileInfo(int wd, const char * name, const Functor && cb):
        wd_(wd), name_(name), cb_(std::move(cb))
    {}

    int wd() { return wd_; }
    std::string & name() { return name_; }
public:
    int wd_;
    std::string name_;
    Functor cb_;
};

}

class FileNotify
{
public:
    typedef std::function<void(const FileNotifyImpl::FileInfo)> Functor;

    FileNotify(EventLoop * loop);
    ~FileNotify();

    bool addWatch(const char * name, const Functor && cb);
    void rmWatch(const char * name);

 private:
    void onRead();
    static void read_cb(struct bufferevent * bev, void * ctx);
    bool read(void * data, size_t datlen);

private:
    EventLoop *            loop_;
    int                      notifyFd_;
    struct bufferevent *    bufev_;
    std::map<int, FileNotifyImpl::FileInfo> notifyMap_;
    std::map<std::string, int> fileMap_;
};

#endif
