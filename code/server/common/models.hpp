#pragma once
#include <iostream>

namespace chat_ns
{
    struct User
    {
        unsigned long long id;   // bigint unsigned
        std::string user_id;      // varchar(64)
        std::string nickname;    // varchar(64)
        std::string description; // text
        std::string password;    // varchar(64)
        std::string phone;       // varchar(64)
        std::string avatar_id;    // varchar(64)
    };

    struct ChatSession
    {
        unsigned long long id;           // bigint unsigned
        std::string chat_session_id;     // varchar(64)
        std::string chat_session_name;   // varchar(64)
        unsigned char chat_session_type; // tinyint
    };

    struct SingleChatSession
    {
        std::string chat_session_id;
        std::string friend_id;
    };

    struct GroupChatSession {
        std::string chat_session_id;
        std::string chat_session_name;
    };

    struct ChatSessionMember
    {
        unsigned long long id;  // BIGINT UNSIGNED
        std::string session_id; // varchar(64)
        std::string user_id;    // varchar(64)
    };

    struct FriendApply
    {
        unsigned long long id; // BIGINT UNSIGNED
        std::string event_id;  // varchar(64)
        std::string user_id;   // varchar(64)
        std::string peer_id;   // varchar(64)
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
    };

    struct Relation
    {
        unsigned long long id; // BIGINT UNSIGNED
        std::string user_id;   // varchar(64)
        std::string peer_id;   // varchar(64)
    };
}