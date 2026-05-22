#ifndef USER_H
#define USER_H

#include <string>

class User {
public:
    int id{-1};

    std::string username;
    std::string password;
    std::string salt;

    User();
    User(
        int id_,
        const std::string& user,
        const std::string& pass,
        const std::string& salt_
        );
};

#endif
