#include "FileNotify.h"

#include <unistd.h>
#include <sys/inotify.h>

#include "BaseUtil.h"
#include "EventLoop.h"

FileNotify::FileNotify(EventLoop * loop):
    _loop(loop),
    _notifyFd(inotify_init1(IN_NONBLOCK|IN_CLOEXEC)),
    _bufev(bufferevent_socket_new(_loop->get_event(), _notifyFd, BEV_OPT_CLOSE_ON_FREE))
{
    bufferevent_setcb(_bufev, read_cb, nullptr, nullptr, this);
    bufferevent_enable(_bufev, EV_READ|EV_WRITE|EV_PERSIST|EV_ET);
}

FileNotify::~FileNotify()
{
    bufferevent_free(_bufev);
    _notifyFd = -1;
}

bool FileNotify::addWatch(const char * name, const Functor && cb)
{
    _loop->assertInLoopThread();
    int wd = inotify_add_watch(_notifyFd, name, IN_MODIFY);
    if(wd == -1)
    {
        LOG_ERROR("read errno=%d, error:%s", name, strerror(errno));
        _fileMap.erase(name);
        return false;
    }

    FileNotifyImpl::FileInfo info(wd, name, std::move(cb));
    _notifyMap.insert(std::make_pair(wd, info));
    _fileMap.insert(std::make_pair(name, wd));
    return true;
}

void FileNotify::rmWatch(const char * name)
{
    _loop->assertInLoopThread();
    auto it = _fileMap.find(name);
    if(it != _fileMap.end())
    {
        _notifyMap.erase(it->second);
        inotify_rm_watch(_notifyFd, it->second);
        _fileMap.erase(it);
    }
}

void FileNotify::onRead()
{
    struct inotify_event et;
    while(true)
    {
        if(!read(&et, sizeof(et)))
        {
            break;
        }

        auto it = _notifyMap.find(et.wd);
        if(it == _notifyMap.end())
        {
            continue;
        }

        const FileNotifyImpl::FileInfo & info = it->second;
        if(info._cb)
        {
            info._cb(info);
        }

        if(et.mask & IN_IGNORED)
        {
            addWatch(info._name.c_str(), std::move(info._cb));
            _notifyMap.erase(et.wd);
        }
    }
}

void FileNotify::read_cb(struct bufferevent * bev, void * ctx)
{
    NOTUSED_ARG(bev);
    static_cast<FileNotify *>(ctx)->onRead();
}

bool FileNotify::read(void * data, size_t datlen)
{
    _loop->assertInLoopThread();
    assert(_bufev != nullptr);

    struct evbuffer * inputBuffer = bufferevent_get_input(_bufev);
    if(!inputBuffer)
    {
        LOG_ERROR("read errno=%d, error:%s", errno, strerror(errno));
        return false;
    }

    int  inputLen = evbuffer_get_length(inputBuffer);
    if(inputLen < static_cast<int>(datlen))
    {
        //input buffer not enough
        return false;
    }

    ASSERT_ABORT(bufferevent_read(_bufev, data, datlen) == datlen);
    return true;
}
