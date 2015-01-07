#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

namespace LibSys
{

int daemonize(int nochdir, int noclose)
{
    int fd;

    switch (fork()) {
    case -1:
        return (-1);
    case 0:
        break;
    default:
        _exit(EXIT_SUCCESS);
    }

    if (setsid() == -1)
        return (-1);

    if (nochdir == 0) {
        if(chdir("/") != 0) {
            perror("chdir");
            return (-1);
        }
    }

    if (noclose == 0 && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        if(dup2(fd, STDIN_FILENO) < 0) {
            return (-1);
        }
        if(dup2(fd, STDOUT_FILENO) < 0) {
            return (-1);
        }
        if(dup2(fd, STDERR_FILENO) < 0) {
            return (-1);
        }

        if (fd > STDERR_FILENO) {
            if(close(fd) < 0) {
                return (-1);
            }
        }
    }
    return (0);
}

int daemon_lock(const char* fpath)
{
    if(fpath == NULL)
        return -1;

    int fd = open(fpath, O_RDWR | O_CREAT, 0644);
    if(fd < 0)
        return -1;

    struct flock fl;
    pid_t pid = getpid();

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = pid;

    if(fcntl(fd, F_SETLK, &fl) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            return -1;
        }

        return -2;
    }

    // convert pid to string and write to fd 
    char buf[16];
    int n = 0;
    while(pid > 0 && n < 16)
    {
        // convert to ascii
        buf[15 - n] = (pid % 10) + 48;
        pid /= 10;
        ++n;
    }

    if(n > 0)
    {
        ftruncate(fd, 0);
        write(fd, buf + 16 - n, n);
    }

    return fd;
}

} // namespace LibSys
