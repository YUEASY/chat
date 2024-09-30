#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "utils.hpp"
#include "models.hpp"
#include <memory>

namespace chat_ns
{

    class UserTable
    {   // +-------------+-----------------+------+-----+---------+----------------+
        // | Field       | Type            | Null | Key | Default | Extra          |
        // +-------------+-----------------+------+-----+---------+----------------+
        // | id          | bigint unsigned | NO   | PRI | NULL    | auto_increment |
        // | user_id     | varchar(64)     | NO   | UNI | NULL    |                |
        // | nickname    | varchar(64)     | YES  | UNI | NULL    |                |
        // | description | text            | YES  |     | NULL    |                |
        // | password    | varchar(64)     | YES  |     | NULL    |                |
        // | phone       | varchar(64)     | YES  | UNI | NULL    |                |
        // | avatar_id   | varchar(64)     | YES  |     | NULL    |                |
        // +-------------+-----------------+------+-----+---------+----------------+
    public:
        using ptr = std::shared_ptr<UserTable>;

        UserTable()
        {
            mysql = Utils::mysqlInit(DB_NAME, HOST, PORT, USER, PASSWD);
            if (mysql == nullptr)
            {
                exit(-1);
            }
        }
        ~UserTable()
        {
            Utils::mysqlDestroy(mysql);
        }

        bool getUserByNickname(std::string_view nickname, User &user)
        {
            std::string sql = "SELECT * FROM users WHERE nickname = '";
            sql.append(nickname);
            sql.append("';");

            mtx.lock();
            if (!Utils::mysqlQuery(mysql, sql))
            {
                mtx.unlock();
                return false;
            }

            MYSQL_RES *res = mysql_store_result(mysql);
            if (res == nullptr)
            {
                LOG_ERROR("mysql store result error: " + std::string(mysql_error(mysql)));
                mtx.unlock();
                return false;
            }
            mtx.unlock();

            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                user.id = std::stoull(row[0]);
                user.user_id = row[1];
                user.nickname = row[2];
                user.description = row[3] ? row[3] : "";
                user.password = row[4];
                user.phone = row[5];
                user.avatar_id = row[6] ? row[6] : "";
                mysql_free_result(res);
                return true;
            }

            mysql_free_result(res);
            return false;
        }

        bool getUserByPhone(std::string_view phone, User &user)
        {
            std::string sql = "SELECT * FROM users WHERE phone = '";
            sql.append(phone);
            sql.append("';");

            mtx.lock();
            if (!Utils::mysqlQuery(mysql, sql))
            {
                mtx.unlock();
                return false;
            }

            MYSQL_RES *res = mysql_store_result(mysql);
            if (res == nullptr)
            {
                LOG_ERROR("mysql store result error: " + std::string(mysql_error(mysql)));
                mtx.unlock();
                return false;
            }
            mtx.unlock();

            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                user.id = std::stoull(row[0]);
                user.user_id = row[1];
                user.nickname = row[2];
                user.description = row[3] ? row[3] : "";
                user.password = row[4];
                user.phone = row[5];
                user.avatar_id = row[6] ? row[6] : "";
                mysql_free_result(res);
                return true;
            }

            mysql_free_result(res);
            return false;
        }

        bool getUserById(std::string_view id, User &user)
        {
            std::string sql = "SELECT * FROM users WHERE user_id = '";
            sql.append(id);
            sql.append("';");

            mtx.lock();
            if (!Utils::mysqlQuery(mysql, sql))
            {
                mtx.unlock();
                return false;
            }

            MYSQL_RES *res = mysql_store_result(mysql);
            if (res == nullptr)
            {
                LOG_ERROR("mysql store result error: " + std::string(mysql_error(mysql)));
                mtx.unlock();
                return false;
            }
            mtx.unlock();

            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                user.id = std::stoull(row[0]);
                user.user_id = row[1];
                user.nickname = row[2];
                user.description = row[3] ? row[3] : "";
                user.password = row[4];
                user.phone = row[5];
                user.avatar_id = row[6] ? row[6] : "";
                mysql_free_result(res);
                return true;
            }

            mysql_free_result(res);
            return false;
        }

        bool getUsersById(const std::vector<std::string> &ids, std::unordered_map<std::string, User> &users)
        {
            if (ids.empty())
            {
                return false; // 如果没有传入ID，直接返回false
            }

            // 构建SQL查询
            std::string sql = "SELECT * FROM users WHERE user_id IN (";
            for (size_t i = 0; i < ids.size(); ++i)
            {
                sql.append("'").append(ids[i]).append("'");
                if (i < ids.size() - 1)
                {
                    sql.append(", ");
                }
            }
            sql.append(");");

            mtx.lock();
            if (!Utils::mysqlQuery(mysql, sql))
            {
                mtx.unlock();
                return false;
            }

            MYSQL_RES *res = mysql_store_result(mysql);
            if (res == nullptr)
            {
                LOG_ERROR("mysql store result error: " + std::string(mysql_error(mysql)));
                mtx.unlock();
                return false;
            }
            mtx.unlock();

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.id = std::stoull(row[0]);           // id (bigint unsigned)
                user.user_id = row[1];                    // user_id (varchar(64))
                user.nickname = row[2];                  // nickname (varchar(64))
                user.description = row[3] ? row[3] : ""; // description (text)
                user.password = row[4];                  // password (varchar(64))
                user.phone = row[5];                     // phone (varchar(64))
                user.avatar_id = row[6] ? row[6] : "";    // avatar_id (varchar(64))

                users[user.user_id] = user; // 将用户添加到哈希表中
            }

            mysql_free_result(res);
            return true;
        }

        bool createUser(const User &user)
        {
            std::string sql;
            sql.append("INSERT INTO users (user_id, nickname, description, password, phone, avatar_id) VALUES ('");
            sql.append(user.user_id);
            sql.append("', '");
            sql.append(user.nickname);
            sql.append("', '");
            sql.append(user.description);
            sql.append("', '");
            sql.append(user.password);
            sql.append("', '");
            sql.append(user.phone);
            sql.append("', '");
            sql.append(user.avatar_id);
            sql.append("');");

            mtx.lock();
            bool result = Utils::mysqlQuery(mysql, sql);
            mtx.unlock();

            return result;
        }

        bool updateUserInfo(const User &user)
        {
            std::string sql;
            sql.append("UPDATE users SET nickname = '");
            sql.append(user.nickname);
            sql.append("', description = '");
            sql.append(user.description);
            sql.append("', password = '");
            sql.append(user.password);
            sql.append("', phone = '");
            sql.append(user.phone);
            sql.append("', avatar_id = '");  
            sql.append(user.avatar_id);
            sql.append("' WHERE user_id = '");
            sql.append(user.user_id);
            sql.append("';");

            mtx.lock();
            bool result = Utils::mysqlQuery(mysql, sql);
            mtx.unlock();

            return result;
        }

    private:
        MYSQL *mysql; // 一个对象就是一个客户端，管理一张表
        std::mutex mtx;

        const char *HOST = "127.0.0.1";
        const char *PORT = "3306";
        const char *USER = "root";
        const char *PASSWD = "123456";
        const char *DB_NAME = "chat";
    };

} // namespace chat_ns
