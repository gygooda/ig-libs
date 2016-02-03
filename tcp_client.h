#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <string>
#include <ctime>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "utils.h"
#include "log.h"

using std::string;

/**
 * TCP client封装类，利用阻塞模式连接到server。
 * 使用本类一定要定时检测socket是否关闭（调用reconnnect,或is_real_alive函数），
 * 以保证socket能及时关闭，否则socket在被动关闭时(server关闭socket)，
 * client socket会长时间处于close_wait状态。
 */
class TcpClient
{
public:
    enum
    {
        TC_ERROR = -1,
        TC_TIMEOUT = 0,
        TC_OK = 1
    };

    TcpClient() : m_fd(-1), m_ip(""), m_port(0), m_timeout(3000000)
    {}
    TcpClient(const string& ip, int port, time_t timeout) : m_fd(-1), m_ip(ip), m_port(port), m_timeout(timeout)
    {}
    virtual ~TcpClient()
    {
        close();
    }

    int connect(); // 利用ip地址建立连接
    int connect(const string& host, const string& port);
    int reconnect();
    int send(const void* buf, size_t len) const;
    int recv(void* buf, size_t len) const;
    int writev(const struct iovec *iov, int iovcnt) const;
    int readv(const struct iovec *iov, int iovcnt) const;
    void close() const;

    // 调用recv()系统函数测试socket是否alive
    bool is_real_alive() const;
    // 仅测试fd是否存在.
    bool is_alive() const;
    /**
     * param: 超时时间，注意以秒为单位。
     */
    int set_timeout(time_t t);

    int get_ip_port(uint32_t& ip, uint16_t& port) const;
private:
    int connect_host(); // 先解析域名，得到ip：port，然后连接。

private:
    mutable int m_fd;
    string m_str_host; // 域名地址
    string m_ip; // 由域名解析出来到ip地址
    string m_str_port;
    int m_port;

    time_t m_timeout;

    // disable copy constructor
    TcpClient(const TcpClient& tc);
    TcpClient& operator = (const TcpClient& tc);
};

inline int TcpClient::reconnect()
{
    if(is_real_alive() == false)
    {
        DEBUG_LOG("disconnected to %s:%d, try to reconnect.", m_ip.c_str(), m_port);
        this->close();
        this->connect_host();
    }

    return 0;
}

inline int TcpClient::connect(const string& host, const string& port)
{
    m_str_host = host;
    m_str_port = port;

    return connect_host();
}

inline bool TcpClient::is_alive() const
{
    return (m_fd < 0) ? false : true;
}

inline void TcpClient::close() const
{
    if(m_fd >= 0)
    {
        ::shutdown(m_fd, SHUT_RDWR);

        if(::close(m_fd) != 0)
            ERROR_LOG("close socket failed: %s", strerror(errno));
        m_fd = -1;
    }
}
#endif
