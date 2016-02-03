#include "log.h"
#include "proto_dispatcher.h"

// 成功返回0，失败返回非0，>0表示proto_id已经存在，<0表示handler为NULL。
int ProtoDispatcher::register_proto(ProtoId proto_id, ProtoHandler* handler)
{
    if(likely(handler != NULL))
    {
        if(handler->init() != 0)
        {
            ERROR_LOG("proto id 0x%X, handler: %p, init failed.", proto_id, handler);
            return -1;
        }

        HandlerMgr::iterator it = m_handler_mgr.find(proto_id);
        if(it != m_handler_mgr.end())
        {
            ERROR_LOG("register proto 0x%X failed: already exists.", proto_id);
            handler->uninit();
            return 1;
        }

        m_handler_mgr.insert(std::make_pair(proto_id, handler));
        DEBUG_LOG("proto id 0x%X register SUCC.", proto_id);
        return 0;
    }

    ERROR_LOG("add proto 0x%X for NULL handler %p.", proto_id, handler);
    return -1;
}

int ProtoDispatcher::unregister_proto(ProtoId proto_id)
{
    HandlerMgr::iterator it = m_handler_mgr.find(proto_id);
    if(it != m_handler_mgr.end() && it->second)
    {
        it->second->uninit();

        m_handler_mgr.erase(it);
        DEBUG_LOG("proto id 0x%X unregister SUCC.", proto_id);
        return 0;
    }

    ERROR_LOG("unregister proto 0x%X failed.", proto_id);
    return -1;
}

int ProtoDispatcher::dispatch(ProtoId proto_id, int fd, void* buf, uint32_t len)
{
    HandlerMgr::iterator it = m_handler_mgr.find(proto_id);
    if(it != m_handler_mgr.end() && it->second)
    {
        if(unlikely(it->second->process(fd, buf, len) == PHR_PRINT_PROTOS))
        {
            print_supported_protocols();
        }

        return 0;
    }

    return -1;
}

void ProtoDispatcher::timer_event()
{
    for(HandlerMgr::iterator it = m_handler_mgr.begin(); it != m_handler_mgr.end(); ++it)
    {
        if(it->second)
            it->second->timer_event(it->first);
    }
}

void ProtoDispatcher::timer_event(ProtoId proto_id)
{
    HandlerMgr::iterator it = m_handler_mgr.find(proto_id);
    if(it != m_handler_mgr.end() && it->second)
    {
        if(it != m_handler_mgr.end() && it->second)
            it->second->timer_event(proto_id);
    }
}

void ProtoDispatcher::clear_protos()
{
    for(HandlerMgr::iterator it = m_handler_mgr.begin(); it != m_handler_mgr.end(); )
    {
        ProtoId id = it->first;
        ++it;
        unregister_proto(id);
    }
    
    m_handler_mgr.clear();
}

void ProtoDispatcher::print_supported_protocols() const
{
    DEBUG_LOG("===================================================");
    DEBUG_LOG("=                Supported Protocols              =");

    for(HandlerMgr::const_iterator it = m_handler_mgr.begin(); it != m_handler_mgr.end(); ++it)
    {
        if(it->second)
        {
            DEBUG_LOG("\t0x%X\t%s\t%p", it->first, it->second->get_proto_name().c_str(), it->second);
        }
    }

    DEBUG_LOG("===================================================");
}
