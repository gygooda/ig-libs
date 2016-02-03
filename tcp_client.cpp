#include <cstdlib>

#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "string_utils.h"
#include "tcp_client.h"

int TcpClient::connect()
{
    if(m_fd >= 0)
        return m_fd;

    if(m_ip.empty() || m_port < 0 || m_port > 65535)
        return TC_ERROR;

    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_fd == TC_ERROR)
    {
        ERROR_LOG("create socket failed: %s", strerror(errno));
        return TC_ERROR;
    }

    do
    {
        struct timeval tv = {0,0};
        tv.tv_sec = m_timeout / 1000000;
        tv.tv_usec = m_timeout % 1000000;

        if(setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0 ||
           setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0)
        {
            ERROR_LOG("set socket timeout failed: %s", strerror(errno));
            break;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(m_port);
        if(inet_pton(AF_INET, m_ip.c_str(), &serv_addr.sin_addr) != 1)
        {
            ERROR_LOG("inet_pton %s failed: %s", m_ip.c_str(), strerror(errno));
            break;
        }

        int ret = ::connect(m_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if(ret == 0)
            return m_fd;
        else if(ret == -1)
        {
            if(errno == EINPROGRESS)
                ERROR_LOG("select timeout while tring to connect to %s: %s", m_ip.c_str(), strerror(errno));
            else
                ERROR_LOG("connect to %s failed: %s", m_ip.c_str(), strerror(errno));
            break;
        }
    }
    while(0);

    this->close();
    return TC_ERROR;
}

int TcpClient::connect_host()
{
    if(m_str_host.empty() || m_str_port.empty() || BossCommon::is_all_digit(m_str_port) == false)
        return TC_ERROR;

    m_port = atoi(m_str_port.c_str());

    if(m_port < 0 || m_port > 65535)
    {
        ERROR_LOG("port %d out of bound.", m_port);
        return TC_ERROR;
    }

    // 支持域名
    struct addrinfo hints;
    struct addrinfo *results, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;    
    hints.ai_flags = 0;

    int s = 0;
    if((s = getaddrinfo(m_str_host.c_str(), m_str_port.c_str(), &hints, &results)) != 0)
    {
        ERROR_LOG("getaddrinfo from %s:%d, %s\n", m_str_host.c_str(), m_port, gai_strerror(s));
        return TC_ERROR;
    }   

    for (rp = results; rp != NULL; rp = rp->ai_next)
    {
        char dst_ip[16] = {0};

        if(inet_ntop(AF_INET, &((struct sockaddr_in *)(rp->ai_addr))->sin_addr, dst_ip, sizeof(dst_ip)) == NULL)
        {
            ERROR_LOG("inet_ntop get ip failed: %s", strerror(errno));
            return TC_ERROR;
        }

        m_ip = dst_ip;

        if(this->connect() != TC_ERROR)
        {
            DEBUG_LOG("connected to %s:%s, resolved ip: %s, port: %d.", m_str_host.c_str(), m_str_port.c_str(),
                    dst_ip, m_port);
            break;
        }
        else
        {
            ERROR_LOG("connect to %s:%s failed, resolved ip: %s, port: %d.", m_str_host.c_str(), m_str_port.c_str(),
                    dst_ip, m_port);
        }
    }

    if (rp == NULL)
    {
        ERROR_LOG("connect to %s:%s failed.", m_str_host.c_str(), m_str_port.c_str());
        freeaddrinfo(results);
        return -1;
    }

    freeaddrinfo(results);
    return m_fd;
}

/** 
 * @brief: 发送指定数量的数据。只要发送成功，发送的数据大小是参数len.
 * @return: 失败时返回-1，超时返回0，成功返回发送的数据大小。
 */
int TcpClient::send(const void* buf, size_t len) const
{
    if(m_fd < 0 || buf == NULL || len == 0)
        return TC_ERROR;

    ssize_t sent_bytes = 0;

    while(1)
    {
        ssize_t r = ::send(m_fd, (char*)buf + sent_bytes, len - sent_bytes, 0);
        if(r < 0)
        {
            // 发送超时.
            // async_server接收到协议包片段时，是一直取到完整包为止。
            // 当第一次发送便超时，则返回超时。如果已经发送了一部分，
            // 则关闭socket，下次重新连接，发送，这样async_server才会释放
            // 收到的片段。
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if(sent_bytes == 0)
                {
                    ERROR_LOG("timeout while sending data.");
                    return TC_TIMEOUT;
                }
            }
            else if(errno == EINTR)
                continue;

            ERROR_LOG("send failed: %s.", strerror(errno));
            this->close();
            return TC_ERROR;
        }

        sent_bytes += r;
        if((size_t)sent_bytes >= len)
            break;
    }

    return len;
}

/**
 * @return: 成功时返回发送的数据长度， 超时返回0， 失败时返回-1.
 * 注意：发送的数据长度可能小于请求的数据长度，调用该函数后需要检查
 * 是否完整发送。
 */
int TcpClient::writev(const struct iovec *iov, int iovcnt) const
{
    if(m_fd < 0 || iov == NULL || iovcnt == 0)
        return TC_ERROR;

    ssize_t sent_bytes = ::writev(m_fd, iov, iovcnt);
    if(sent_bytes <= 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ERROR_LOG("timeout while sending data.");
            return TC_TIMEOUT;
        }

        ERROR_LOG("writev failed: %s.", strerror(errno));
        this->close();
        return TC_ERROR;
    }

    return sent_bytes;
}

/**
 * @return: 成功时返回收到的数量长度， 超时返回0， 失败时返回-1.
 * 注意：接收的数据长度可能小于请求的数据长度，调用该函数后需要检查
 * 是否完整接收。
 */
int TcpClient::readv(const struct iovec *iov, int iovcnt) const
{
    if(m_fd < 0 || iov == NULL || iovcnt == 0)
        return TC_ERROR;

    ssize_t recv_bytes = ::readv(m_fd, iov, iovcnt);
    if(recv_bytes == 0)
    {
        ERROR_LOG("socket close while receiving data.");
        this->close();
        return TC_ERROR;
    }
    else if(recv_bytes < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ERROR_LOG("timeout while receiveing data.");
            return TC_TIMEOUT;
        }

        ERROR_LOG("readv failed: %s.", strerror(errno));
        this->close();
        return TC_ERROR;
    }

    return recv_bytes;
}

/** 
 * @brief: 接收指定数量的数据。只要接收成功，接收的数据大小是参数len.
 * @return: 失败时返回-1，超时返回0，成功返回接收的数据大小。
 */
int TcpClient::recv(void* buf, size_t len) const
{
    if(m_fd < 0 || buf == NULL || len == 0)
        return TC_ERROR;

    ssize_t recv_bytes = 0;

    while(1)
    {
        ssize_t r = ::recv(m_fd, (char*)buf + recv_bytes, len - recv_bytes, 0);
        if(r == 0)
        {
            ERROR_LOG("socket close while receiving data.");
            this->close();
            return TC_ERROR;
        }
        else if(r < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                ERROR_LOG("timeout while receiveing data.");
                return TC_TIMEOUT;
            }
            else if(errno == EINTR)
            {
                continue;
            }

            ERROR_LOG("recv failed: %s.", strerror(errno));
            this->close();
            return TC_ERROR;
        }

        recv_bytes += r;
        if((size_t)recv_bytes >= len)
            break;
    }

    return len;
}

bool TcpClient::is_real_alive() const
{
    if(m_fd < 0)
        return false;

    char test[1];

    ssize_t ret = ::recv(m_fd, test, 1, MSG_PEEK | MSG_DONTWAIT);

    if(ret > 0)
        return true;
    else if(ret == 0)
    {
        return false;
    }
    else
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            return true;
        else
        {
            return false;
        }
    }
}

int TcpClient::set_timeout(time_t t)
{
    m_timeout = t * 1000000;

    struct timeval tv = {0, 0};
    tv.tv_sec = m_timeout / 1000000;
    tv.tv_usec = m_timeout % 1000000;

    if(m_fd >= 0)
    {
        if(setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0 ||
                setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0)
        {
            ERROR_LOG("set socket timeout failed: %s", strerror(errno));
            return TC_ERROR; // -1
        }
    }

    return 0; // succuss
}

// 返回网络字节序的ip和port
int TcpClient::get_ip_port(uint32_t& ip, uint16_t& port) const
{
    if(m_fd < 0)
        return -1;

    struct sockaddr_in sock_addr;
    socklen_t sock_len = sizeof(sock_addr);

    if(getsockname(m_fd, (struct sockaddr *)&sock_addr, &sock_len) < 0)
        return -1;

    ip = sock_addr.sin_addr.s_addr;
    port = sock_addr.sin_port;

    return 0;
}
