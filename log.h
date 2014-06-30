#ifndef IG_LIBS_SYS_LOG_H
#define IG_LIBS_SYS_LOG_H

#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <deque>
#include <string>
#include <pthread.h>
#include <sys/time.h>

#define IG_LOG_LEVEL_ERROR 0
#define IG_LOG_LEVEL_USER_ERROR  1
#define IG_LOG_LEVEL_WARN  2
#define IG_LOG_LEVEL_INFO  3
#define IG_LOG_LEVEL_TRACE 4
#define IG_LOG_LEVEL_DEBUG 5

#define IG_LOG_LEVEL(level) IG_LOG_LEVEL_##level, __FILE__, __LINE__, __FUNCTION__, pthread_self()
#define IG_LOG_NUM_LEVEL(level) level, __FILE__, __LINE__, __FUNCTION__, pthread_self()
#define IG_LOGGER IgSys::IgLogger::get_logger()

#define IGSYS_PRINT(level, ...) IG_LOGGER.log_message(IG_LOG_LEVEL(level), __VA_ARGS__)
#define IG_LOG_BASE(level, ...) (IG_LOG_LEVEL_##level>IG_LOGGER._level) ? (void)0 : IGSYS_PRINT(level, __VA_ARGS__)
#define IG_LOG(level, _fmt_, args...) ((IG_LOG_LEVEL_##level>IG_LOGGER._level) ? (void)0 : IG_LOG_BASE(level,_fmt_, ##args))
#define IG_LOG_US(level, _fmt_, args...) \
  ((IG_LOG_LEVEL_##level>IG_LOGGER._level) ? (void)0 : IG_LOG_BASE(level, "[%ld][%ld][%ld] " _fmt_, \
                                                            pthread_self(), IgSys::IgLogger::get_cur_tv().tv_sec, \
                                                            IgSys::IgLogger::get_cur_tv().tv_usec, ##args))

namespace IgSys 
{
using std::deque;
using std::string;

class IgLogger 
{
public:
    static const mode_t LOG_FILE_MODE = 0644;
    IgLogger();
    ~IgLogger();

    void rotate_log(const char *filename, const char *fmt = NULL);
    void log_message(int level, const char *file, int line, const char *function, pthread_t tid, const char *fmt, ...) __attribute__ ((format (printf, 7, 8)));
    /**
     * @brief set log putout level
     *
     * @param level DEBUG|WARN|INFO|TRACE|ERROR
     *
     * @param wf_level set the level output to wf log file
     */
    void set_log_level(const char *level, const char *wf_level = NULL);
    /**
     * @brief set log file name
     *
     * @param filename log file name
     *
     * @param flag whether to redirect stdout to log file, if false, redirect it
     *
     * @param open_wf whether to open wf log file, default close
     */
    void set_file_name(const char *filename, bool flag = false, bool open_wf = false);
    void check_file();
    void set_check(int v) {_check = v;}
    void set_max_file_size( int64_t maxFileSize=0x40000000);
    void set_max_file_index( int maxFileIndex= 0x0F);

    static inline struct timeval get_cur_tv()
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      return tv;
    };

    static IgLogger& get_logger();

private:
    int _fd;
    int _wf_fd;
    char *_name;
    int _check;
    size_t _maxFileIndex; // 至程序退出，最大的日志文件个数，超过该值，最早访问的日志文件将被删除。
    int64_t _maxFileSize; // 每个日志文件的最大大小。
    bool _flag;
    bool _wf_flag;
    std::deque<std::string> _fileList;
    std::deque<std::string> _wf_file_list;
    static const char *const _errstr[];
    pthread_mutex_t _fileSizeMutex;
    pthread_mutex_t _fileIndexMutex;

public:
    int _level;
    int _wf_level;

private:
    IgLogger(const IgLogger&);
    IgLogger& operator = (const IgLogger&);
};

}
#endif
