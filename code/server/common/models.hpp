#include <iostream>

namespace chat_ns
{
    struct User
    {
        unsigned long long id;   // bigint unsigned
        std::string userId;      // varchar(64)
        std::string nickname;    // varchar(64)
        std::string description; // text
        std::string password;    // varchar(64)
        std::string phone;       // varchar(64)
        std::string avatarId;    // varchar(64)
    };
}