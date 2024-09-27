#include "icsearch.hpp"
#include "models.hpp"

namespace chat_ns
{
    class ESClientFactory
    {
    public:
        static std::shared_ptr<elasticlient::Client> create(const std::vector<std::string> host_list)
        {
            return std::make_shared<elasticlient::Client>(host_list);
        }
    };
    class ESUser
    {
    public:
        using ptr = std::shared_ptr<ESUser>;
        ESUser(const std::shared_ptr<elasticlient::Client> &client) : _es_client(client) {}
        bool createIndex()
        {
            bool ret = ESIndex(_es_client, "user")
                           .append("user_id", "keyword", "standard", true)
                           .append("nickname")
                           .append("phone", "keyword", "standard", true)
                           .append("description", "text", "standard", false)
                           .append("avatar_id", "keyword", "standard", false)
                           .create();
            if (ret == false)
            {
                LOG_INFO("用户信息索引创建失败!");
                return false;
            }
            LOG_INFO("用户信息索引创建成功!");
            return true;
        }
        bool appendData(const std::string &uid,
                        const std::string &phone,
                        const std::string &nickname,
                        const std::string &description,
                        const std::string &avatar_id)
        {
            bool ret = ESInsert(_es_client, "user")
                           .append("user_id", uid)
                           .append("nickname", nickname)
                           .append("phone", phone)
                           .append("description", description)
                           .append("avatar_id", avatar_id)
                           .insert(uid);
            if (ret == false)
            {
                LOG_ERROR("用户数据插入/更新失败!");
                return false;
            }
            LOG_INFO("用户数据新增/更新成功!");
            return true;
        }
        std::vector<User> search(const std::string &key, const std::vector<std::string> &uid_list)
        {
            std::vector<User> res;
            Json::Value json_user = ESSearch(_es_client, "user")
                                        .appendShouldMatch("phone.keyword", key)
                                        .appendShouldMatch("user_id.keyword", key)
                                        .appendShouldMatch("nickname", key)
                                        .appendMustNotTerm("user_id.keyword", uid_list)
                                        .search();
            if (json_user.isArray() == false)
            {
                LOG_ERROR("用户搜索结果为空，或者结果不是数组类型");
                return res;
            }
            int sz = json_user.size();
            LOG_DEBUG("检索结果条目数量：{}", sz);
            for (int i = 0; i < sz; i++)
            {
                User user;
                user.userId = json_user[i]["_source"]["user_id"].asString();
                user.nickname = json_user[i]["_source"]["nickname"].asString();
                user.description = json_user[i]["_source"]["description"].asString();
                user.phone = json_user[i]["_source"]["phone"].asString();
                user.avatarId = json_user[i]["_source"]["avatar_id"].asString();
                res.push_back(user);
            }
            return res;
        }

    private:
        // const std::string _uid_key = "user_id";
        // const std::string _desc_key = "user_id";
        // const std::string _phone_key = "user_id";
        // const std::string _name_key = "user_id";
        // const std::string _avatar_key = "user_id";
        std::shared_ptr<elasticlient::Client> _es_client;
    };

    class ESMessage
    {
    public:
        using ptr = std::shared_ptr<ESMessage>;
        ESMessage(const std::shared_ptr<elasticlient::Client> &es_client) : _es_client(es_client) {}
        bool createIndex()
        {
            bool ret = ESIndex(_es_client, "message")
                           .append("user_id", "keyword", "standard", false)
                           .append("message_id", "keyword", "standard", false)
                           .append("create_time", "long", "standard", false)
                           .append("chat_session_id", "keyword", "standard", true)
                           .append("content")
                           .create();
            if (ret == false)
            {
                LOG_INFO("消息信息索引创建失败!");
                return false;
            }
            LOG_INFO("消息信息索引创建成功!");
            return true;
        }
        bool appendData(const std::string &user_id,
                        const std::string &message_id,
                        const std::string &create_time,
                        const std::string &chat_session_id,
                        const std::string &content)
        {
            bool ret = ESInsert(_es_client, "message")
                           .append("message_id", message_id)
                           .append("create_time", create_time)
                           .append("user_id", user_id)
                           .append("chat_session_id", chat_session_id)
                           .append("content", content)
                           .insert(message_id);
            if (ret == false)
            {
                LOG_ERROR("消息数据插入/更新失败!");
                return false;
            }
            LOG_INFO("消息数据新增/更新成功!");
            return true;
        }
        bool remove(const std::string &mid)
        {
            bool ret = ESRemove(_es_client, "message").remove(mid);
            if (ret == false)
            {
                LOG_ERROR("消息数据删除失败!");
                return false;
            }
            LOG_INFO("消息数据删除成功!");
            return true;
        }
    private:
        std::shared_ptr<elasticlient::Client> _es_client;
    };
}