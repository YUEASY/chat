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
}