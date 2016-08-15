#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <vector>
#include <memory>

class Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;
#define MakeBufferPtr std::make_shared<Buffer>

class Buffer
{
public:
    Buffer() {}
    virtual ~Buffer() {}
public:
    char * data() { return _buf.data(); }
    char * data(size_t len) { return _buf.data()+len; }

    void clear() { _buf.clear(); }
    bool empty() { return _buf.empty(); }
    size_t size() { return _buf.size(); }
    void resize(size_t len) { _buf.resize(len); }
    void reserve(size_t len) { _buf.reserve(len); }

    size_t append(const void * buf, size_t len);
private:
    std::vector<char> _buf;
};

#endif
