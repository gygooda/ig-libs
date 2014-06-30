#ifndef IG_LIBS_SYS_RING_BUFFER_H
#define IG_LIBS_SYS_RING_BUFFER_H

#include <sys/mman.h>

namespace IgSys
{

// 无锁的循环队列，只支持一个写线程和一个读线程的情况。
// 由linux kernel 2.6.32修改而来, 增加支持mmap分配内存。
// 如果用于进程间共享，则需在ring_buffer_alloc()前调用set_shared(true), 否则只能用于线程。
class RingBuffer
{
public:
    RingBuffer() : m_buffer(NULL), m_size(0), m_in(0), m_out(0), m_shared(false)
    {}
    ~RingBuffer()
    {
        ring_buffer_free();
    }

    unsigned long ring_buffer_alloc(unsigned long size);
    void ring_buffer_free();
    unsigned long ring_buffer_write(const unsigned char* buffer, unsigned long len);
    unsigned long ring_buffer_read(unsigned char* buffer, unsigned long len);

    void ring_buffer_reset()
    {
        m_in = m_out = 0;
    }

    // return capacity of the ring buffer.
    unsigned long ring_buffer_size() const
    {
        return m_size;
    }
    // return bytes we can read from the ring buffer.
    unsigned long ring_buffer_readable_size() const
    {
        return m_in - m_out;
    }

    bool get_shared() const
    {
        return m_shared;
    }

    void set_shared(bool shared)
    {
        if(m_buffer)
            return; // 如果内存空间已经分配，则不能改变其共享属性。
        m_shared = shared;
    }

private:
    // disable copy constructors
    RingBuffer(const RingBuffer&);
    RingBuffer& operator = (const RingBuffer&);

private:
    unsigned char *m_buffer;  /* the buffer holding the data */
    unsigned long m_size;  /* the size of the allocated buffer */
    unsigned long m_in;    /* data is added at offset (m_in % m_size) */
    unsigned long m_out;   /* data is extracted from off. (m_out % m_size) */

    bool m_shared;
};

} // namespace IgSys
#endif
