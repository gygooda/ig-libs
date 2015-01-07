#include <cstring>
#include <algorithm>
#include <cstdlib>

#include "utils.h"
#include "ring_buffer.h"

namespace LibSys
{

/**
 * return 0, if alloc fails.
 * return allocated size if succ.
 */
unsigned long RingBuffer::ring_buffer_alloc(unsigned long size)
{
    if(!is_power_of_2(size))
    {
        if((size = roundup_pow_of_two(size)) == 0)
            return 0;
    }

    if(!m_shared)
    {
        if(posix_memalign(reinterpret_cast<void**>(&m_buffer), 16, size) != 0)
        {
            m_buffer = NULL;
            return 0;
        }
    }
    else
    {
        m_buffer = static_cast<unsigned char*>(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
        if(m_buffer == MAP_FAILED)
        {
            m_buffer = NULL;
            return 0;
        }
    }

    m_size = size;
    ring_buffer_reset();

    return size;
}

void RingBuffer::ring_buffer_free()
{
    if(!m_shared)
        free(m_buffer);
    else
        munmap(m_buffer, m_size);

    ring_buffer_reset();
    m_buffer = NULL;
}

unsigned long RingBuffer::ring_buffer_write(const unsigned char* buffer, unsigned long len)
{
    unsigned long l;

    len = std::min(len, m_size - m_in + m_out);

    /*
     * Ensure that we sample the m_out index -before- we
     * start putting bytes into the ring buffer.
     */
    mb();

    /* first put the data starting from m_in to buffer end */
    l = std::min(len, m_size - (m_in & (m_size - 1)));
    memcpy(m_buffer + (m_in & (m_size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(m_buffer, buffer + l, len - l);

    /*
     * Ensure that we add the bytes to the ring buffer -before-
     * we update the m_in index.
     */
    wmb();

    m_in += len;

    return len;
}

unsigned long RingBuffer::ring_buffer_read(unsigned char* buffer, unsigned long len)
{
    unsigned long l;

    len = std::min(len, m_in - m_out);

    /*
     * Ensure that we sample the m_in index -before- we
     * start removing bytes from the ring buffer.
     */

    rmb();

    /* first get the data from m_out until the end of the buffer */
    l = std::min(len, m_size - (m_out & (m_size - 1)));
    memcpy(buffer, m_buffer + (m_out & (m_size - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, m_buffer, len - l);

    /*
     * Ensure that we remove the bytes from the ring buffer -before-
     * we update the m_out index.
     */
    mb();

    m_out += len;

    return len;
}

} // namespace LibSys
