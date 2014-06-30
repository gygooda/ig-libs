#ifndef IG_LIBS_SYS_SHARED_QUEUE_H
#define IG_LIBS_SYS_SHARED_QUEUE_H

#include <unistd.h>
#include <cstdint>

#include "ring_buffer.h"

namespace IgSys
{

class SharedQueue
{
public:
    SharedQueue()
    {
        _reset_fd();
    }
    ~SharedQueue()
    {
        _close_fd();
    }

    // size必须是2的指数
    int init(size_t size, bool shared = false);
    int uninit();

    int get_read_fd() const
    {
        return _read_fd();
    }

    size_t get_size() const
    {
        return m_ring_buffer.ring_buffer_size();
    }
    size_t get_writeable_size() const
    {
        return get_size() - get_readable_size();
    }
    size_t get_readable_size() const
    {
        return m_ring_buffer.ring_buffer_readable_size();
    }

    ssize_t read(void *buf, size_t size, bool clear_notify = true);
    ssize_t write(const void *buf, size_t size, bool write_notify = true);

private:
    int m_pipefd[2];
    int _read_fd() const
    {
        return m_pipefd[0];
    }
    int _write_fd() const
    {
        return m_pipefd[1];
    }
    void _reset_fd()
    {
        m_pipefd[0] = -1;
        m_pipefd[1] = -1;
    }
    void _close_fd()
    {
        if(_read_fd() >= 0)
            ::close(_read_fd());
        if(_write_fd() >= 0)
            ::close(_write_fd());
        _reset_fd();
    }

    RingBuffer m_ring_buffer;
};

} // namespace IgSys

#endif
