#ifndef IG_LIBS_SYS_DAEMON_H
#define IG_LIBS_SYS_DAEMON_H

namespace LibSys
{
    int daemonize(int nochdir, int noclose);

    /**
     * 新建daemon lock（文件锁），用于实现daemon只启动一个实例。
     * 成功, 返回文件描述符, 该文件描述符不能被关闭，除非进程退出。
     * 失败，返回-1
     */
    int daemon_lock(const char* fpath);
}

#endif
