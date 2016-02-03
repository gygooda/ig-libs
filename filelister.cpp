#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "filelister.h"

FileLister::FileLister(const std::string& path)	: m_dirpath(path), m_dir(NULL)
{
}

FileLister::~FileLister()
{
	close();
}

void FileLister::close()
{
    if(m_dir)
    {
        closedir(m_dir);
        m_dir = NULL;
    }
}

int FileLister::open(const std::string& path)
{
	m_dirpath = path;
    return this->open();
}

int FileLister::open()
{
    close();

	m_dir = opendir(m_dirpath.c_str());
    if(m_dir != NULL)
        return 0;

    return -1;
}

void FileLister::start()
{
    if(m_dir)
        rewinddir(m_dir);
}

bool FileLister::next(string &filename)
{
    if(NULL == m_dir)
        return false;

	dirent* entry = readdir(m_dir);
	while (entry) 
    {
		if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN)
        {
            filename = entry->d_name;
			return true;
		}

		entry = readdir(m_dir);
	}

	return false;
}
