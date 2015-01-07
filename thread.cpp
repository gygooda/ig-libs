#include "thread.h"

namespace LibSys
{

void* Thread::thread_func(void * arg)
{
    Thread* self = static_cast<Thread*>(arg);
    long ret = self->thread_run();

    self->thread_post_run();

    pthread_exit(reinterpret_cast<void*>(ret));

    return NULL;
}

int Thread::run(void)
{
    if(thread_pre_run() < 0)
        return -1;

    if(pthread_create(&m_tid, NULL, thread_func, (void*)this) != 0)
        return -2;

    return 0;
}

} // namespace LibSys
