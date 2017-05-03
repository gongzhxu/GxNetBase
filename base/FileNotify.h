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
        _wd(0)
    {}

    FileInfo(int wd, const char * name, const Functor && cb):
        _wd(wd), _name(name), _cb(std::move(cb))
    {}

    int wd() { return _wd; }
    std::string & name() { return _name; }
public:
    int _wd;
    std::string _name;
    Functor _cb;
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
