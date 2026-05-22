#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <sqlite3.h>
#include <optional>
#include <vector>
#include <string>

#include "User.h"
#include "Device.h"
#include "Event.h"

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool init(const std::string& filename = "iot.db");

    bool create_user(
        const std::string& username,
        const std::string& password,
        std::string& out_err
        );

    std::optional<User> verify_user(
        const std::string& username,
        const std::string& password,
        std::string& out_err
        );

    bool add_device(const Device& d, std::string& out_err);

    std::vector<Device> get_devices(
        int user_id,
        std::string& out_err
        );

    bool delete_device(
        int id,
        int user_id,
        std::string& out_err
        );

    bool update_status(
        int device_id,
        const std::string& status,
        std::string& out_err
        );

    bool add_event(
        int device_id,
        const std::string& event,
        std::string& out_err
        );

    std::vector<Event> get_events(
        int device_id,
        std::string& out_err
        );

private:
    sqlite3* db_;

    bool execute_sql(
        const std::string& sql,
        std::string& out_err
        );

    static std::string generate_salt(size_t bytes = 16);
    static std::string sha256_hex(const std::string& input);
    static std::string h_password(
        const std::string& password,
        const std::string& salt
        );
};

#endif
