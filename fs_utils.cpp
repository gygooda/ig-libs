#include <cassert>
#include <cerrno>
#include <iomanip>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>

#include "string_utils.h"
#include "fs_utils.h"
#include "filelister.h"

using std::string;

namespace BossCommon
{
    bool makedir(const string& path)
    {
        assert(path.size());

        string dir;
        vector<string> subdirs;
        split(path, '/', subdirs);

        if(subdirs.size() <= 0)
            return false;

        if (subdirs[0].empty()) {
            subdirs.erase(subdirs.begin());
        } else if (subdirs[0] == ".") {
            dir = ".";
            subdirs.erase(subdirs.begin());
        } else if (subdirs[0] == "..") {
            dir = "..";
            subdirs.erase(subdirs.begin());
        } else {
            dir = ".";
        }

        for (vector<string>::size_type i = 0; i != subdirs.size(); ++i) {
            dir += "/" + subdirs[i];
            int ret = mkdir(dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            if ((ret == -1) && (errno != EEXIST)) {
                return false;
            }
        }

        return true;
    }

    int get_dir_files_size(const std::string& path, uint32_t& file_count, uint64_t& total_size, uint32_t max_count)
    {
        uint32_t count = 0;
        uint64_t path_size = 0;

        FileLister filelister(path);
        filelister.open();
        filelister.start();

        string file_name;
        while(filelister.next(file_name))
        {
            ++count;

            struct stat file_stat;
            string file_path = path + "/" + file_name;
            if(::stat(file_path.c_str(), &file_stat) != 0)
                return -1;

            path_size += file_stat.st_size;

            if(count >= max_count) // 只扫描max_count次，避免文件数多而耗时
                break;
        }

        file_count = count;
        total_size = path_size;

        return 0;
    }

    int get_dir_free_size(const std::string& path, uint64_t& free_size)
    {
        if(path.empty())
            return -1;

        struct stat blk_stat;
        struct statfs fs_stat;

        if(::stat(path.c_str(), &blk_stat) != 0)
            return -1;

        uint32_t blk_size = blk_stat.st_blksize;

        if(::statfs(path.c_str(), &fs_stat) != 0)
            return -1;

        free_size = blk_size * fs_stat.f_bavail;

        return 0;
    }

    std::string convert_disk_size_unit(uint64_t bytes)
    {
        const char *unit[] = {" Bytes", " KB", " MB", " GB", " TB", " PB", " EB", " ZB", " YB"};

        uint8_t count = 0;
        double result = bytes;
        while(bytes >= 1024)
        {
            if(count > sizeof(unit)/sizeof(const char *) - 1)
                break;
            bytes /= 1024;
            result /= 1024.0;
            ++count;
        }

        stringstream ss;

        ss << std::setprecision(2) << std::fixed << result << unit[count];
        return ss.str();
    }
}
