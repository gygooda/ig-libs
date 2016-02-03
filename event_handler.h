#pragma once

/**
 * 事件处理接口。
 */
class EventHandler
{
public:
    EventHandler(int fd, void* arg) : m_fd(fd), m_arg(arg)
    {}
    EventHandler() : m_fd(-1), m_arg(NULL)
    {}
    virtual ~EventHandler()
    {}

    void set_event_args(int fd, void* arg)
    {
        m_fd = fd;
        m_arg = arg;
    }

    int handle_write_event()
    {
        return write_handler(m_fd, m_arg);
    }
    int handle_read_event()
    {
        return read_handler(m_fd, m_arg);
    }
    int handle_error_event()
    {
        return error_handler(m_fd, m_arg);
    }
    int handle_close_event()
    {
        return close_handler(m_fd, m_arg);
    }

private:
    virtual int read_handler(int fd, void* arg) = 0;
    virtual int write_handler(int fd, void* arg) = 0;
    virtual int error_handler(int fd, void* arg) = 0;
    virtual int close_handler(int fd, void* arg) = 0;

private:
    int m_fd;
    void* m_arg;
};
