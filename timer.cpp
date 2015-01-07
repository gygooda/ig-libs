#include "timer.h"

namespace LibSys
{

int TimerManager::add_timer(unsigned itv, ITimerCallback* cb, void* cb_arg)
{
    if(m_timer_list.size() >= MAX_TIME_ENVET_COUNT)
        return -1;

    TimerEvent te(itv, cb, cb_arg);
    if(!te.is_valid())
        return -1;

    m_timer_list.push_back(te);

    return 0;
}

int TimerManager::remove_timer(unsigned itv, ITimerCallback* cb, void* cb_arg)
{
    TimerEvent te(itv, cb, cb_arg);

    if(!te.is_valid())
        return -1;

    m_timer_list.remove(te);

    return 0;
}

void TimerManager::clear()
{
    m_timer_list.clear();
}

int TimerManager::process_timer(time_t now)
{
    if(now == 0)
        now = time(0);

    TimeEventList::iterator it;
    for(it = m_timer_list.begin(); it != m_timer_list.end();)
    {
        if((*it).process_timer_event(now) == 1)
        {
            it = m_timer_list.erase(it);
        }
        else
            ++it;
    }

    return 0;
}

} // namespace LibSys
