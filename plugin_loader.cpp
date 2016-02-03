#include <dlfcn.h>

#include "log.h"
#include "plugin_loader.h"

// 成功返回0，失败返回负数，其中该so已存在返回-2.
int PluginLoader::load(const PathType& path)
{
    if(unlikely(path.empty()))
        return -1;

    PluginMap::iterator it = m_plugins.find(path);
    if(it != m_plugins.end())
    {
        ERROR_LOG("plugin %s already exists.", path.c_str());
        return -2;
    }

    void* plugin = dlopen(path.c_str(), RTLD_LAZY);
    if(plugin == NULL)
    {
        ERROR_LOG("load %s failed: %s", path.c_str(), dlerror());
        return -1;
    }

    PluginIface* plugin_iface = (PluginIface*)dlsym(plugin, "g_plugin_iface");
    if(plugin_iface == NULL)
    {
        ERROR_LOG("dlsym g_plugin_iface failed: %s", dlerror());
        dlclose(plugin);
        return -1;
    }

    plugin_iface->plugin_so = plugin;
    if(plugin_iface->init && plugin_iface->register_proto)
    {
        if(plugin_iface->init() != 0)
        {
            ERROR_LOG("init plugin %s failed.", path.c_str());
            dlclose(plugin);
            return -1;
        }
        if(plugin_iface->register_proto(&m_dispatcher) != 0)
        {
            ERROR_LOG("plugin %s register proto failed.", path.c_str());
            if(plugin_iface->uninit)
                plugin_iface->uninit();
            dlclose(plugin);
            return -1;
        }
    }
    else
    {
        dlclose(plugin);
        ERROR_LOG("error in so, no init or register_proto function found.");
        return -1;
    }

    m_plugins.insert(std::make_pair(path, plugin_iface));
    return 0;
}

// 返回成功加载so的个数
int PluginLoader::load(const PathListType& paths)
{
    int ret = 0;
    for(PathListType::const_iterator it = paths.begin(); it != paths.end(); ++it)
    {
        load(*it);
        ++ret;
    }

    return ret;
}

int PluginLoader::unload(const PathType& path)
{
    if(unlikely(path.empty()))
        return -1;

    int ret = 0;
    PluginMap::iterator it = m_plugins.find(path);
    if(it != m_plugins.end() && it->second)
    {
        void * plugin = it->second->plugin_so;
        if(plugin)
        {
            if(it->second->unregister_proto)
                it->second->unregister_proto(&m_dispatcher);

            if(it->second->uninit)
                it->second->uninit();

            // FIXME: dlclose失败后，没办法处理
            if(dlclose(plugin) != 0)
            {
                ret = -1;
                ERROR_LOG("dlclose %s failed: %s.", path.c_str(), dlerror());
            }
        }
    }

    m_plugins.erase(path);
    return ret;
}

// 返回成功卸载so的个数
int PluginLoader::unload(const PathListType& paths)
{
    int ret = 0;
    for(PathListType::const_iterator it = paths.begin(); it != paths.end(); ++it)
    {
        unload(*it);
        ++ret;
    }

    return ret;
}

void PluginLoader::clear_plugins()
{
    PluginMap::iterator it = m_plugins.begin();
    while(it != m_plugins.end())
    {
        PathType path = it->first;
        ++it;
        unload(path);
    }
}

