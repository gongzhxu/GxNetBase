#include "Buffer.h"
#include "string.h"

size_t Buffer::append(const void * buf, size_t len)
{
    size_t oldlen = size();
    resize(oldlen+len);

    memcpy(data(oldlen), buf, len);
    return len;
}
