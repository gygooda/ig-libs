#include "singleton.h"
#include <new>

Singleton* Singleton::m_instance = NULL;
pthread_mutex_t Singleton::m_lock = PTHREAD_MUTEX_INITIALIZER;

Singleton* Singleton::GetInstance()
{
    if(m_instance == NULL)
    {
        pthread_mutex_lock(&m_lock);
        if(m_instance == NULL)
        {
            m_instance = new (std::nothrow) Singleton();
        }
        pthread_mutex_unlock(&m_lock);
    }

    return m_instance;
}
