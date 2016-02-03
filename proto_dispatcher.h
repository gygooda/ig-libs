#ifndef PROTO_DISPATCHER_H
#define PROTO_DISPATCHER_H

#include <unordered_map>

#include "utils.h"
#include "proto_handler.h"

class ProtoDispatcher
{
public:
    ProtoDispatcher()
    {}
    ~ProtoDispatcher()
    {
        clear_protos();
    }

    int register_proto(ProtoId proto_id, ProtoHandler* handler);
    int unregister_proto(ProtoId proto_id);
    void clear_protos();

    int dispatch(ProtoId proto_id, int fd, void* buf, uint32_t len);
    void timer_event();
    void timer_event(ProtoId proto_id);

    void print_supported_protocols() const;

private:
    typedef std::unordered_map<ProtoId, ProtoHandler*> HandlerMgr;

    HandlerMgr m_handler_mgr;

    DISALLOW_COPY_AND_ASSIGN(ProtoDispatcher);
};

#endif
