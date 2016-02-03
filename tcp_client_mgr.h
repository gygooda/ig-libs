#ifndef TCP_CLIENT_MGR_HPP
#define TCP_CLIENT_MGR_HPP

#include <vector>
#include <string>

#include "tcp_client.h"

using std::vector;
using std::string;

class TcpClientMgr
{
public:
    TcpClientMgr();
    ~TcpClientMgr();

    // host_list, ip:port，以;（分号）分割多个ip:port
    // send_type，0：任意发给其中一个ip，1：同时发给所有ip
    int init(const string& host_list, int send_type = 0, time_t timeout = 2);
    int uninit();

    int check_connection();
    int select_connection() const;
    void close_connection(unsigned int conn_id) const;

    /**
     * 该函数返回指向连接的指针，和select_connection语义相同，
     * 而且会破坏类的封装性，但的确可以减少函数调用次数，
     * 提高性能。
     * 注意：切勿对返回的指针做delete操作！
     */
    TcpClient* get_available_connection() const;

    /**
     * send和recv函数可能会调用频繁，所以采用inline.
     * 不带conn_id参数的，为自动选择，每次选择的可能不一样，
     * 所以不适合一个包分多次发送时调用。
     * 带conn_id参数时，为手工选择，在调用前，先调用select_connection.
     *
     * 注意：本系列函数未实现send_type语义。
     */
    int send(unsigned int conn_id, const void* buf, size_t len) const;
    int send(const void* buf, size_t len) const;
    int recv(unsigned int conn_id, void* buf, size_t len) const;
    int recv(void* buf, size_t len) const;
    int writev(unsigned int conn_id, const struct iovec *iov, int iovcnt) const;
    int writev(const struct iovec *iov, int iovcnt) const;
    int readv(unsigned int conn_id, const struct iovec *iov, int iovcnt) const;
    int readv(const struct iovec *iov, int iovcnt) const;

    /**
     * 发送请求并等待返回，该函数依赖协议包头定义，并且只适合只返回包头的的协议。
     * 目前实现依赖boss的统一协议包头，成功时返回包头中的status字段(为正整数)，
     * 发送或接收失败时, 返回-1.
     * 实现send_type语义。
     */
    int send_request(const void* buf, size_t len);

private:
    typedef vector<TcpClient *> TcpClientMap;
    TcpClientMap m_connections;
    unsigned int m_conn_count; // 连接的个数，用于减少对m_connections.size()的调用。
    mutable unsigned int m_conn_index; // 一个自增的序号，用于轮转获取连接，简单的负载均衡。

    int m_send_type; // 0：任意发给其中一个ip，1：同时发给所有ip 

    // disable copy constructors
    TcpClientMgr(const TcpClientMgr& slc);
    TcpClientMgr& operator = (const TcpClientMgr& slc);
};

inline int TcpClientMgr::send(unsigned int conn_id, const void* buf, size_t len) const
{
    if(conn_id >= m_conn_count)
        return -2;

    return m_connections[conn_id]->send(buf, len);
}

inline int TcpClientMgr::send(const void* buf, size_t len) const
{
    int conn_id = select_connection();
    if(conn_id < 0)
        return -1;

    return this->send((unsigned int)conn_id, buf, len);
}

inline int TcpClientMgr::recv(unsigned int conn_id, void* buf, size_t len) const
{
    if(conn_id >= m_conn_count)
        return -2;

    return m_connections[conn_id]->recv(buf, len);
}

inline int TcpClientMgr::recv(void* buf, size_t len) const
{
    int conn_id = select_connection();
    if(conn_id < 0)
        return -1;

    return this->recv((unsigned int)conn_id, buf, len);
}

inline int TcpClientMgr::writev(unsigned int conn_id, const struct iovec *iov, int iovcnt) const
{
    if(conn_id >= m_conn_count)
        return -2;

    return m_connections[conn_id]->writev(iov, iovcnt);
}

inline int TcpClientMgr::writev(const struct iovec *iov, int iovcnt) const
{
    int conn_id = select_connection();
    if(conn_id < 0)
        return -1;

    return this->writev((unsigned int)conn_id, iov, iovcnt);
}

inline int TcpClientMgr::readv(unsigned int conn_id, const struct iovec *iov, int iovcnt) const
{
    if(conn_id >= m_conn_count)
        return -2;

    return m_connections[conn_id]->readv(iov, iovcnt);
}

inline int TcpClientMgr::readv(const struct iovec *iov, int iovcnt) const
{
    int conn_id = select_connection();
    if(conn_id < 0)
        return -1;

    return this->readv((unsigned int)conn_id, iov, iovcnt);
}

inline void TcpClientMgr::close_connection(unsigned int conn_id) const
{
    if(conn_id >= m_conn_count)
        return;

    m_connections[conn_id]->close();
}

#endif
