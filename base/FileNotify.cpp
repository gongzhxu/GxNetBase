#include "FileNotify.h"

#include <unistd.h>
#include <sys/inotify.h>

#include "BaseUtil.h"
#include "EventLoop.h"

FileNotify::FileNotify(EventLoop * loop):
    loop_(loop),
    notifyFd_(inotify_init1(IN_NONBLOCK|IN_CLOEXEC)),
    bufev_(bufferevent_socket_new(loop_->get_event(), notifyFd_, BEV_OPT_CLOSE_ON_FREE))
{
    bufferevent_setcb(bufev_, read_cb, nullptr, nullptr, this);
    bufferevent_enable(bufev_, EV_READ|EV_WRITE|EV_PERSIST|EV_ET);
}

FileNotify::~FileNotify()
{
    bufferevent_free(bufev_);
    notifyFd_ = -1;
}

bool FileNotify::addWatch(const char * name, const Functor && cb)
{
    loop_->assertInLoopThread();
    int wd = inotify_add_watch(notifyFd_, name, IN_MODIFY);
    if(wd == -1)
    {
        LOG_ERROR("read errno=%d, error:%s", name, strerror(errno));
        fileMap_.erase(name);
        return false;
    }

    FileNotifyImpl::FileInfo info(wd, name, std::move(cb));
    notifyMap_.insert(std::make_pair(wd, info));
    fileMap_.insert(std::make_pair(name, wd));
    return true;
}

void FileNotify::rmWatch(const char * name)
{
    loop_->assertInLoopThread();
    auto it = fileMap_.find(name);
    if(it != fileMap_.end())
    {
        notifyMap_.erase(it->second);
        inotify_rm_watch(notifyFd_, it->second);
        fileMap_.erase(it);
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

        auto it = notifyMap_.find(et.wd);
        if(it == notifyMap_.end())
        {
            continue;
        }

        const FileNotifyImpl::FileInfo & info = it->second;
        if(info.cb_)
        {
            info.cb_(info);
        }

        if(et.mask & IN_IGNORED)
        {
            addWatch(info.name_.c_str(), std::move(info.cb_));
            notifyMap_.erase(et.wd);
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
    loop_->assertInLoopThread();
    assert(bufev_ != nullptr);

    struct evbuffer * inputBuffer = bufferevent_get_input(bufev_);
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

    ASSERT_ABORT(bufferevent_read(bufev_, data, datlen) == datlen);
    return true;
}
