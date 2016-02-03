#ifndef PROTO_HANDLER_H
#define PROTO_HANDLER_H

#include <cstdint>
#include <string>

typedef uint16_t ProtoId;

enum ProtoHandlerRet
{
    PHR_PRINT_PROTOS = 100
};

// 协议处理基类，所有协议处理类继承该类
class ProtoHandler
{
public:
    ProtoHandler(ProtoId proto_id, const std::string& proto_name) : m_proto_id(proto_id), m_proto_name(proto_name)
    {
    }
    virtual ~ProtoHandler()
    {
    }

    virtual int init() = 0;
    virtual int uninit() = 0;
    virtual int process(int fd, void* buf, uint32_t len) = 0;
    virtual void timer_event(ProtoId proto_id)
    {}

    ProtoId get_proto_id() const
    {
        return m_proto_id;
    }

    const std::string& get_proto_name() const
    {
        return m_proto_name;
    }

private:
    ProtoId m_proto_id;
    std::string m_proto_name;
};

#endif
