#include <vector>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>

#include "utils.h"
#include "string_utils.h"
#include "tcp_client_mgr.h"
#include "log.h"

TcpClientMgr::TcpClientMgr() : m_conn_count(0), m_conn_index(0), m_send_type(0)
{
}

TcpClientMgr::~TcpClientMgr()
{
    uninit();
}

/**
 * @param host_list, 一台服务器以格式：域名:端口 配置，多台服务器用分号(;)分隔, 
 *                   比如：223.32.2.1:3304;234.343.231.3:3305
 * @param timeout, 设置网络超时
 */
int TcpClientMgr::init(const string& host_list, int send_type, time_t timeout)
{
    if(m_conn_count > 0) // 已经初始化过了
        return 0;
    if(host_list.empty())
        return -1;

    m_send_type = send_type;
    if(m_send_type != 0 && m_send_type != 1)
        m_send_type = 1;

    DEBUG_LOG("init tcp client from host string: %s", host_list.c_str());
    vector<string> addr_vec;
    BossCommon::split(host_list, ';', addr_vec);
    for(unsigned i = 0; i < addr_vec.size(); ++i)
    {
        DEBUG_LOG("parsing host string: %s", addr_vec[i].c_str());
        vector<string> ip_port;
        BossCommon::split(addr_vec[i], ':', ip_port);
        if(ip_port.size() != 2)
        {
            ERROR_LOG("illgal ip port: %s", addr_vec[i].c_str());
            continue;
        }

        DEBUG_LOG("add host: ip: %s, port: %s", ip_port[0].c_str(), ip_port[1].c_str());
        TcpClient* tc = new (std::nothrow) TcpClient();
        if(tc == NULL)
        {
            ERROR_LOG("new TcpClient failed.");
            continue;
        }

        tc->set_timeout(timeout);
        tc->connect(ip_port[0], ip_port[1]);

        m_connections.push_back(tc);
        ++m_conn_count;
    }

    //if(m_conn_count == 0)
    //{
        //ERROR_LOG("cannot connect to any host.");
        //return -1;
    //}

    return 0;
}

int TcpClientMgr::uninit()
{
    for(unsigned int i = 0; i < m_conn_count; ++i)
    {
        delete m_connections[i];
    }

    m_connections.clear();
    m_conn_count = 0;

    m_conn_index = 0;

    return 0;
}

int TcpClientMgr::check_connection()
{
    for(unsigned int i = 0; i < m_conn_count; ++i)
    {
        m_connections[i]->reconnect();
    }

    return 0;
}

int TcpClientMgr::select_connection() const
{
    if(m_conn_count == 0)
    {
        return -1;
    }

    /*
     * 简单的负载平衡调度，轮转。
     */
    for(unsigned int i = 0; i < m_conn_count; ++i)
    {
        uint32_t id = m_conn_index++ % m_conn_count;
        if(m_connections[id]->is_alive())
            return id;
    }

    ERROR_LOG("ALARM: all connections were lost.");
    return -1;
}

/** 
 * @brief: 获取一个连接指针，和select_connection语义相同，
 * 而且会破坏类的封装性，但的确可以减少函数调用次数，
 * 提高性能。
 * 注意：切勿对返回的指针做delete操作！
 * @return: 返回一个连接指针，失败时返回NULL。
 */
TcpClient* TcpClientMgr::get_available_connection() const
{
    if(m_conn_count == 0)
    {
        return NULL;
    }

    /*
     * 简单的负载平衡调度，轮转。
     */
    for(unsigned int i = 0; i < m_conn_count; ++i)
    {
        uint32_t id = m_conn_index++ % m_conn_count;
        if(m_connections[id]->is_alive())
            return m_connections[id];
    }
    
    ERROR_LOG("ALARM: all connections were lost.");
    return NULL;
}

/**
 * 发送请求并等待返回，该函数依赖协议包头定义，并且只适合只返回包头的的协议。
 * 目前实现依赖boss的统一协议包头，成功时返回包头中的status字段(正整数)，
 * 发送或接收失败时, 返回-1，当m_send_type = 1时，任意ip发送失败，返回-1。
 */
/*
int TcpClientMgr::send_request(const void* buf, size_t len)
{
    int count = 1;
    if(m_send_type != 0)
        count = m_conn_count;

    int ret = 0;
    TcpClient* tc = NULL;
    for(int i = 0; i < count; ++i)
    {
        tc = get_available_connection();
        if(unlikely(tc == NULL))
        {
            return -1;
        }

        ret = tc->send(buf, len);
        if(unlikely(ret < 0 || (size_t)ret != len))
        {
            ret = -1;
            break;
        }

        BossProtoHeader h;
        ret = tc->recv(&h, sizeof(h));
        if(unlikely(ret < 0))
        {
            ret = -1;
            break;
        }

        ret = h.status;
    }

    if(unlikely(ret < 0))
    {
        tc->close();
        tc->reconnect();
    }

    return ret;
}
*/
