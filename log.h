#include "log_base.h"

/**
 * 用于服务器后台程序写日志。
 * 所有日志将写到一个文件中。
 * 线程安全，可以在多线程，多进程环境中使用。
 */

LibSys::Logger& get_logger();

#define LOGGER get_logger()
#define LOG_PRINT(level, ...) LOGGER.log_message(LOG_LEVEL(level), __VA_ARGS__)
#define LOG_BASE(level, ...) (LOG_LEVEL_##level>LOGGER._level) ? (void)0 : LOG_PRINT(level, __VA_ARGS__)
#define LOG_US(level, _fmt_, args...) \
  ((LOG_LEVEL_##level>LOGGER._level) ? (void)0 : LOG_BASE(level, "[%ld][%ld][%ld] " _fmt_, \
                                                            pthread_self(), LibSys::Logger::get_cur_tv().tv_sec, \
                                                            LibSys::Logger::get_cur_tv().tv_usec, ##args))

#define LOG(level, _fmt_, args...) ((LOG_LEVEL_##level>LOGGER._level) ? (void)0 : LOG_BASE(level,_fmt_, ##args))
#define ERROR_LOG(_fmt_, args...) LOG(ERROR, _fmt_, ##args)
#define USER_ERROR_LOG(_fmt_, args...) LOG(USER_ERROR, _fmt_, ##args)
#define DEBUG_LOG(_fmt_, args...) LOG(DEBUG, _fmt_, ##args)
#define WARN_LOG(_fmt_, args...) LOG(WARN, _fmt_, ##args)
#define INFO_LOG(_fmt_, args...) LOG(INFO, _fmt_, ##args)
#define TRACE_LOG(_fmt_, args...) LOG(TRACE, _fmt_, ##args)

