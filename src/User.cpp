#include "User.h"

User::User() : id(-1) {}

User::User(
    int id_,
    const std::string& user,
    const std::string& pass,
    const std::string& salt_
    )
{
    id = id_;
    username = user;
    password = pass;
    salt = salt_;
}
