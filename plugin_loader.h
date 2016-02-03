#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <map>
#include <string>
#include <vector>

#include "utils.h"
#include "plugin_iface.h"
#include "proto_dispatcher.h"

class PluginLoader
{
public:
    PluginLoader(ProtoDispatcher& dispatcher) : m_dispatcher(dispatcher)
    {}
    ~PluginLoader()
    {
        clear_plugins();
    }

    typedef std::string PathType;
    typedef std::vector<std::string> PathListType;
    typedef std::map<std::string, PluginIface*> PluginMap;

    int load(const PathType& path);
    int load(const PathListType& paths);

    int unload(const PathType& path);
    int unload(const PathListType& paths);

    void clear_plugins();

private:
    ProtoDispatcher& m_dispatcher;

    PluginMap m_plugins;

    DISALLOW_COPY_AND_ASSIGN(PluginLoader);
};

#endif
