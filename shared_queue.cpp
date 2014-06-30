#include <fcntl.h>

#include "shared_queue.h"

namespace IgSys
{

int SharedQueue::init(size_t size, bool shared)
{
    if(_read_fd() >= 0 || _write_fd() >= 0)
        return 1;

    if(::pipe2(m_pipefd, O_NONBLOCK) < 0)
        return -1;

    m_ring_buffer.set_shared(shared);

    if(m_ring_buffer.ring_buffer_alloc(size) == 0)
    {
        _close_fd();
        return -2;
    }

    return 0;
}

int SharedQueue::uninit()
{
    _close_fd();
    m_ring_buffer.ring_buffer_free();

    return 0;
}

ssize_t SharedQueue::read(void *buf, size_t size, bool clear_notify)
{
    if(get_readable_size() < size)
        return -1;

    unsigned long ret = m_ring_buffer.ring_buffer_read(static_cast<unsigned char*>(buf), size);

    if(ret == 0)
        return -1;

    if(clear_notify)
    {
        char tmp[1] = {0};
        ::read(_read_fd(), tmp, 1);
    }

    return ret;
}

ssize_t SharedQueue::write(const void *buf, size_t size, bool write_notify)
{
    if(get_writeable_size() < size)
        return -1;

    unsigned long ret = m_ring_buffer.ring_buffer_write(static_cast<const unsigned char*>(buf), size);

    if(ret == 0)
        return -1;

    if(write_notify)
    {
        char tmp[1] = {1};
        ::write(_write_fd(), tmp, 1);
    }

    return ret;
}

} // namespace IgSys
