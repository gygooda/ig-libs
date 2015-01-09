#ifndef IG_LIBS_SYS_CONFIG_H
#define IG_LIBS_SYS_CONFIG_H

#include <unordered_map>
#include <string>
#include <sstream>

namespace LibSys
{

/**
 * 配置文件解析模块
 * 支持的配置项格式：
 * key  value
 * key = value
 * key \t value
 * 注意：key和value中不能含有空格，=，\t。
 *
 * 另外：支持include指令，含义为包含另一个配置文件中的配置项
 * 比如：include file/to/include
 */
class Config
{
public:
    Config() : m_file_name(NULL)
    {}
    ~Config()
    {}

    // 从文件中加载配置项
    // 如果overwrite为true，则允许重复的配置项，旧的值将被覆盖。
    // 如果overwrite为false，则不允许重复的配置项，使用最早加载的值。
    // 注意：load和reload非线程安全。
    bool load(const char* file_name, bool overwrite = false);
    // 从文件中加载配置项，已有的配置项会被新值覆盖
    bool reload(const char* file_name);

    // 获取配置项的值
    // 失败时返回default_value
    int get(const std::string& key, int default_value) const
    {
        ConfigHash::const_iterator it = m_config_hash.find(key);
        if(it != m_config_hash.end())
        {
            return atoi(it->second.c_str());
        }

        return default_value;
    }
    float get(const std::string& key, float default_value) const
    {
        ConfigHash::const_iterator it = m_config_hash.find(key);
        if(it != m_config_hash.end())
        {
            return atof(it->second.c_str());
        }

        return default_value;
    }
    const char* get(const std::string& key, const char* default_value) const
    {
        ConfigHash::const_iterator it = m_config_hash.find(key);
        if(it != m_config_hash.end())
        {
            return it->second.c_str();
        }

        return default_value;
    }

private:
    bool parse(const char* file_name, bool overwrite = false);

    // disable copy constructors
    Config(const Config&);
    Config& operator = (const Config&);

private:
    typedef std::unordered_map<std::string, std::string> ConfigHash;

    ConfigHash m_config_hash;

    const char* m_file_name;
};

} // namespace LibSys

#endif
