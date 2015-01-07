#ifndef IG_LIBS_SYS_THREAD_H
#define IG_LIBS_SYS_THREAD_H

#include <pthread.h>

namespace LibSys
{

/**
 * 线程基类，子类实现thread_pre_run(), thread_run(), thread_post_run()即可
 */
class Thread
{
public:
    Thread() : m_tid(0), m_joinable(true)
    {}
    virtual ~Thread()
    {}

    pthread_t thread_id() const
    {
        return m_tid;
    }

    // 启动线程
    // 启动成功, 返回0
    // pre_run失败, 则返回-1
    // 线程启动失败, 返回-2
    int run(void);

    int detach()
    {
        if(!m_joinable)
            return -1;
        int ret = pthread_detach(m_tid);
        if(ret == 0)
            m_joinable = false;
        return ret;
    }
    int join(void** retval) const
    {
        if(!m_joinable)
            return -1;
        return pthread_join(m_tid, retval);
    }

private:
    static void* thread_func(void * arg);

    // run before thread being created.
    // if return value < 0, thread_run(), and thread_post_run() will not be called.
    virtual int thread_pre_run(void) = 0;
    // thread main 
    virtual int thread_run(void) = 0;
    // run after thread exited.
    virtual int thread_post_run(void) = 0;

    // disable copy constructors
    Thread(const Thread&);
    Thread& operator = (const Thread&);

private:
    pthread_t m_tid;
    bool m_joinable;
};

} // namespace LibSys

#endif
