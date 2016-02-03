#ifndef BOSS_FS_UTILS_H
#define BOSS_FS_UTILS_H

#include <string>

namespace BossCommon
{
    // 自动创建path中不存在的子目录
    bool makedir(const std::string& path);

    // 获取目录下文件个数和大小总和，不包括子目录，最多扫描max_count个文件
    int get_dir_files_size(const std::string& path, uint32_t& file_count, uint64_t& total_size, uint32_t max_count = 20000);
    // 获取目录下剩余硬盘大小
    int get_dir_free_size(const std::string& path, uint64_t& free_size);

    // convert bytes to human-readable string
    // 1024 bytes -> 1 KB
    std::string convert_disk_size_unit(uint64_t bytes);
}

#endif
