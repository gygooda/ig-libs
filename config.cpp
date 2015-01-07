#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "log.h"

namespace LibSys
{

bool Config::parse(const char* file_name, bool overwrite)
{
    if(NULL == file_name)
        return false;

    int fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        return false;
    }

    int len = lseek(fd, 0L, SEEK_END);
    lseek(fd, 0L, SEEK_SET);
    char * data = (char *)malloc(len + 1);
    if (data == NULL)
    {
        close(fd);
        return false;
    }

    bool ret = true;
    do
    {
        if (read(fd, data, len) == -1)
        {
            ret = false;
            break;
        }

        data[len] = 0;
        char * start = data;
        char * end;
        while (data + len > start)
        {
            end = strchr(start, '\n');
            if (end)
            {
                *end = 0;
            }

            if (*start != '#')
            {
                char * key;
                char * val;
                key = strtok(start, "= \t");
                val = strtok(NULL, "= \t");
                if (key != NULL && val != NULL)
                {
                    if (strncmp("include", key, 8))
                    {
                        std::string str_key = key;
                        std::string str_val = val;
                        if (m_config_hash.find(str_key) == m_config_hash.end())
                        {
                            m_config_hash[str_key] = str_val;
                        }
                        else
                        {
                            if (overwrite)
                            {
                                m_config_hash[str_key] = str_val;
                            }
                            else
                            {
                                ERROR_LOG("duplicated keys %s in file %s.", key, m_file_name);
                            }
                        }

                    }
                    else
                    {
                        ret = parse(val, overwrite);
                        if (!ret)
                        {
                            break;
                        }
                    }
                }
            }

            if (end)
            {
                start = end + 1;
            }
            else
            {
                break;
            }
        }

    }
    while (0);

    free(data);
    close(fd);

    return ret;
}


bool Config::load(const char * file_name, bool overwrite)
{
    m_file_name = file_name;
    m_config_hash.clear();

    return parse(m_file_name, overwrite);
}

bool Config::reload(const char * file_name)
{
    m_file_name = file_name;

    return parse(m_file_name, true);
}

} // namespace LibSys
