#include <pthread.h>

// thread-safe singleton implementation
class Singleton
{
public:
    ~Singleton()
    {}

    static Singleton* GetInstance();

private:
    Singleton()
    {}
    Singleton(const Singleton&);
    Singleton& operator = (const Singleton&);

    static Singleton* m_instance;
    static pthread_mutex_t m_lock;
};

