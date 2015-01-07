#ifndef IG_LIBS_SYS_TIMER_H
#define IG_LIBS_SYS_TIMER_H

#include <list>
#include <ctime>

using std::list;

namespace LibSys
{

struct ITimerCallback
{
    virtual ~ITimerCallback()
    {}
    // 如果返回1，则定时器会被删除，下次不再执行。
    virtual int timer_callback(void * arg) = 0;
};

// 该类非线程安全
class TimerManager
{
public:
    TimerManager()
    {}
    ~TimerManager()
    {
        clear();
    }

    int add_timer(unsigned interval, ITimerCallback* cb, void* cb_arg);
    int remove_timer(unsigned interval, ITimerCallback* cb, void* cb_arg);

    void clear();

    // 如果参数now = 0, 则调用time(0), 取得当前时间。
    int process_timer(time_t now = 0);

private:
    class TimerEvent
    {
    public:
        TimerEvent() : m_interval(0), m_last_event_time(0), m_callback(NULL), m_callback_arg(NULL)
        {}
        TimerEvent(unsigned interval, ITimerCallback* cb, void * cb_arg) : m_interval(interval), m_last_event_time(0), m_callback(cb), m_callback_arg(cb_arg)
        {}
        ~TimerEvent()
        {}

        bool operator == (const TimerEvent& rhs) const
        {
            if(!is_valid())
                return true;
            return (m_interval == rhs.m_interval && m_callback == rhs.m_callback && m_callback_arg == rhs.m_callback_arg);
        }
        bool operator != (const TimerEvent& rhs) const
        {
            return !operator !=(rhs);
        }

        bool is_valid() const
        {
            return m_interval != 0 && m_callback != NULL;
        }

        int process_timer_event(time_t now)
        {
            if(now - m_last_event_time >= m_interval)
            {
                int ret = m_callback->timer_callback(m_callback_arg);
                m_last_event_time = now;
                return ret;
            }

            return 0;
        }

    private:
        unsigned m_interval;
        time_t m_last_event_time;
        ITimerCallback* m_callback;
        void* m_callback_arg;
    }; // class TimerEvent

private:
    // disable copy constructors
    TimerManager(const TimerManager&);
    TimerManager& operator = (const TimerManager&);

    typedef list<TimerEvent> TimeEventList;
    TimeEventList m_timer_list;

    static const unsigned MAX_TIME_ENVET_COUNT = 1000;
};

} // namespace LibSys

#endif

