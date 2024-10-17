#pragma once
#include <iostream>

namespace chat_ns
{
    struct User
    {
        unsigned long long id;   // bigint unsigned
        std::string user_id;     // varchar(64)
        std::string nickname;    // varchar(64)
        std::string description; // text
        std::string password;    // varchar(64)
        std::string phone;       // varchar(64)
        std::string avatar_id;   // varchar(64)

        User() : id(0), user_id(""), nickname(""), description(""),
                 password(""), phone(""), avatar_id("") {}

        User(const std::string &user_id, const std::string &nickname,
             const std::string &description, const std::string &password,
             const std::string &phone, const std::string &avatar_id)
            : user_id(user_id), nickname(nickname), description(description),
              password(password), phone(phone), avatar_id(avatar_id), id(0) {}
    };

    struct ChatSession
    {
        unsigned long long id;           // bigint unsigned
        std::string chat_session_id;     // varchar(64)
        std::string chat_session_name;   // varchar(64)
        unsigned char chat_session_type; // tinyint

        ChatSession() : id(0), chat_session_id(""),
                        chat_session_name(""), chat_session_type(0) {}

        ChatSession(const std::string &chat_session_id, const std::string &chat_session_name,
                    unsigned char chat_session_type)
            : chat_session_id(chat_session_id), chat_session_name(chat_session_name),
              chat_session_type(chat_session_type), id(0) {}
    };

    struct SingleChatSession
    {
        std::string chat_session_id;
        std::string friend_id;

        SingleChatSession() : chat_session_id(""), friend_id("") {}

        SingleChatSession(const std::string &chat_session_id, const std::string &friend_id)
            : chat_session_id(chat_session_id), friend_id(friend_id) {}
    };

    struct GroupChatSession
    {
        std::string chat_session_id;
        std::string chat_session_name;

        GroupChatSession() : chat_session_id(""), chat_session_name("") {}

        GroupChatSession(const std::string &chat_session_id, const std::string &chat_session_name)
            : chat_session_id(chat_session_id), chat_session_name(chat_session_name) {}
    };

    struct ChatSessionMember
    {
        unsigned long long id;  // BIGINT UNSIGNED
        std::string session_id; // varchar(64)
        std::string user_id;    // varchar(64)

        ChatSessionMember() : id(0), session_id(""), user_id("") {}

        ChatSessionMember(const std::string &session_id, const std::string &user_id)
            : session_id(session_id), user_id(user_id), id(0) {}
    };

    struct FriendApply
    {
        unsigned long long id; // BIGINT UNSIGNED
        std::string event_id;  // varchar(64)
        std::string user_id;   // varchar(64)
        std::string peer_id;   // varchar(64)

        FriendApply() : id(0), event_id(""), user_id(""), peer_id("") {}

        FriendApply(const std::string &event_id, const std::string &user_id,
                    const std::string &peer_id)
            : event_id(event_id), user_id(user_id), peer_id(peer_id), id(0) {}
    };

    struct Message
    {
        unsigned long long id;      // BIGINT UNSIGNED
        std::string message_id;     // varchar(64)
        std::string session_id;     // varchar(64)
        std::string user_id;        // varchar(64)
        unsigned char message_type; // TINYINT UNSIGNED
        std::string create_time;    // TIMESTAMP (可用 string 表示)
        std::string content;        // TEXT (可用 string 表示)
        std::string file_id;        // varchar(64)
        std::string file_name;      // varchar(128)
        unsigned int file_size;     // INT UNSIGNED

        Message() : id(0), message_id(""), session_id(""), user_id(""),
                    message_type(0), create_time(""), content(""),
                    file_id(""), file_name(""), file_size(0) {}

        Message(const std::string &message_id, const std::string &session_id,
                const std::string &user_id, unsigned char message_type,
                const std::string &create_time, const std::string &content,
                const std::string &file_id, const std::string &file_name,
                unsigned int file_size)
            : message_id(message_id), session_id(session_id), user_id(user_id),
              message_type(message_type), create_time(create_time),
              content(content), file_id(file_id), file_name(file_name),
              file_size(file_size), id(0) {}
    };

    struct Relation
    {
        unsigned long long id; // BIGINT UNSIGNED
        std::string user_id;   // varchar(64)
        std::string peer_id;   // varchar(64)

        Relation() : id(0), user_id(""), peer_id("") {}

        Relation(const std::string &user_id, const std::string &peer_id)
            : user_id(user_id), peer_id(peer_id), id(0) {}
    };
}