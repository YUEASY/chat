#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "logger.hpp"

namespace chat_ns
{
    // 定义WebSocket服务器类型，使用websocketpp的ASIO配置
    typedef websocketpp::server<websocketpp::config::asio> server_t;

    class Connection
    {
    public:
        // 定义客户端信息的结构体，包括用户ID（uid）和会话ID（ssid）
        struct Client
        {
            // 构造函数，初始化客户端的用户ID和会话ID
            Client(const std::string &u, const std::string &s) : uid(u), ssid(s) {}
            std::string uid;  // 用户ID
            std::string ssid; // 会话ID
        };
        using ptr = std::shared_ptr<Connection>;

        // 默认构造函数
        Connection() {}

        // 析构函数
        ~Connection() {}

        // 插入新的连接，关联用户ID和会话信息
        // conn：WebSocket连接指针
        // uid：用户ID
        // ssid：会话ID
        void insert(const server_t::connection_ptr &conn,
                    const std::string &uid, const std::string &ssid)
        {
            // 加锁，确保线程安全
            std::unique_lock<std::mutex> lock(_mutex);

            // 将用户ID与连接关联，保存到_uid_connections
            _uid_connections.insert(std::make_pair(uid, conn));

            // 将连接与客户端信息（uid和ssid）关联，保存到_conn_clients
            _conn_clients.insert(std::make_pair(conn, Client(uid, ssid)));

            LOG_DEBUG("新增长连接用户信息：{}-{}-{}", (size_t)conn.get(), uid, ssid);
        }

        // 根据用户ID获取对应的WebSocket连接
        // uid：用户ID
        // 返回：找到的连接，如果未找到则返回空指针
        server_t::connection_ptr connection(const std::string &uid)
        {
            // 加锁，确保线程安全
            std::unique_lock<std::mutex> lock(_mutex);

            // 查找用户ID对应的连接
            auto it = _uid_connections.find(uid);
            if (it == _uid_connections.end())
            {
                LOG_ERROR("未找到 {} 客户端的长连接！", uid);
                // 返回空指针
                return server_t::connection_ptr();
            }

            LOG_DEBUG("找到 {} 客户端的长连接！", uid);

            // 返回找到的连接
            return it->second;
        }

        // 根据连接获取对应的客户端信息（uid和ssid）
        // conn：WebSocket连接指针
        // uid：输出参数，返回的用户ID
        // ssid：输出参数，返回的会话ID
        // 返回：如果找到对应客户端信息，返回true；否则返回false
        bool client(const server_t::connection_ptr &conn, std::string &uid, std::string &ssid)
        {
            // 加锁，确保线程安全
            std::unique_lock<std::mutex> lock(_mutex);

            // 查找连接对应的客户端信息
            auto it = _conn_clients.find(conn);
            if (it == _conn_clients.end())
            {
                LOG_ERROR("获取-未找到长连接 {} 对应的客户端信息！", (size_t)conn.get());
                return false;
            }

            // 找到客户端信息，赋值给输出参数
            uid = it->second.uid;
            ssid = it->second.ssid;

            LOG_DEBUG("获取长连接客户端信息成功！");
            return true;
        }

        // 删除一个连接及其对应的客户端信息
        // conn：要删除的WebSocket连接指针
        void remove(const server_t::connection_ptr &conn)
        {
            // 加锁，确保线程安全
            std::unique_lock<std::mutex> lock(_mutex);

            // 查找连接对应的客户端信息
            auto it = _conn_clients.find(conn);
            if (it == _conn_clients.end())
            {
                LOG_ERROR("删除-未找到长连接 {} 对应的客户端信息！", (size_t)conn.get());
                return;
            }

            // 从_uid_connections中删除该客户端的用户ID关联
            _uid_connections.erase(it->second.uid);

            // 从_conn_clients中删除该连接的关联
            _conn_clients.erase(it);

            LOG_DEBUG("删除长连接信息完毕！");
        }

    private:
        // 互斥锁，用于保护对成员变量的并发访问
        std::mutex _mutex;

        // 保存用户ID到WebSocket连接的映射
        std::unordered_map<std::string, server_t::connection_ptr> _uid_connections;

        // 保存WebSocket连接到客户端信息（Client结构体）的映射
        std::unordered_map<server_t::connection_ptr, Client> _conn_clients;
    };

} // namespace chat_ns
