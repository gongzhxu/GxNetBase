#ifndef buf_FER_H_
#define buf_FER_H_

#include <vector>
#include <memory>

class Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;
#define MakeBufferPtr std::make_shared<Buffer>

/*
   Buffer: extensible buffer block
 */
class Buffer
{
public:
    Buffer() {}
    virtual ~Buffer() {}
public:
    char * data() { return buf_.data(); }
    char * data(size_t len) { return buf_.data()+len; }

    void clear() { buf_.clear(); }
    bool empty() { return buf_.empty(); }
    size_t size() { return buf_.size(); }
    void resize(size_t len) { buf_.resize(len); }
    void reserve(size_t len) { buf_.reserve(len); }

    size_t append(const void * buf, size_t len);
private:
    std::vector<char> buf_;
};

#endif
