#ifndef PLUGIN_IFACE_H
#define PLUGIN_IFACE_H

#include "proto_dispatcher.h"

// 很多业务是经常变化的，比如自定义查询，业务变化，需要改动代码，
// 服务重启，这样会影响已有在线业务。
// 因此设计动态加载plugin(so)的形式来解决该问题。增加或修改业务，
// 只需要加载so即可。每个so调用指定的接口注册协议处理函数，注册成功后，
// 协议处理函数会被自动调用。

// plugin初始化接口
typedef int (*PLUGIN_INIT_FUNC)();
// plugin反初始化接口
typedef int (*PLUGIN_UNINIT_FUNC)();
// plugin注册协议接口
typedef int (*PLUGIN_REG_PROTO_FUNC)(ProtoDispatcher* dispatcher);
// plugin注销协议接口
typedef int (*PLUGIN_UNREG_PROTO_FUNC)(ProtoDispatcher* dispatcher);

typedef struct
{
    void * plugin_so; // dlopen返回的指针, 初始化时应该为NULL
    PLUGIN_INIT_FUNC init;
    PLUGIN_UNINIT_FUNC uninit;
    PLUGIN_REG_PROTO_FUNC register_proto;
    PLUGIN_UNREG_PROTO_FUNC unregister_proto;
}PluginIface;

#endif
