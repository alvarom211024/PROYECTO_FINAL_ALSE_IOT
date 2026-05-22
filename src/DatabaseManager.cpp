#include "DatabaseManager.h"

#include <openssl/sha.h>
#include <openssl/rand.h>

#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <cmath>

DatabaseManager::DatabaseManager() : db_(nullptr) {}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

static std::string sqlite_err(sqlite3* db) {
    return db ? sqlite3_errmsg(db) : "";
}

bool DatabaseManager::init(const std::string& filename) {
    if (sqlite3_open(filename.c_str(), &db_) != SQLITE_OK) {
        return false;
    }

    std::string out_err;
    execute_sql("PRAGMA foreign_keys = ON;", out_err);

    // Esquema normalizado
    // users: autenticación
    // devices: dispositivos IoT por usuario
    // events: historial de eventos por dispositivo
    const char* users_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " username TEXT UNIQUE NOT NULL,"
        " password_hash TEXT NOT NULL,"
        " salt TEXT NOT NULL"
        ");";

    const char* devices_sql =
        "CREATE TABLE IF NOT EXISTS devices ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " user_id INTEGER NOT NULL,"
        " name TEXT NOT NULL,"
        " ip TEXT NOT NULL,"
        " type TEXT NOT NULL,"
        " status TEXT NOT NULL,"
        " interval_time INTEGER NOT NULL DEFAULT 5,"
        " FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");";

    const char* events_sql =
        "CREATE TABLE IF NOT EXISTS events ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " device_id INTEGER NOT NULL,"
        " event TEXT NOT NULL,"
        " datetime TEXT NOT NULL,"
        " FOREIGN KEY(device_id) REFERENCES devices(id) ON DELETE CASCADE"
        ");";

    if (!execute_sql(users_sql, out_err)) return false;
    if (!execute_sql(devices_sql, out_err)) return false;
    if (!execute_sql(events_sql, out_err)) return false;

    return true;
}

bool DatabaseManager::execute_sql(const std::string& sql, std::string& out_err) {
    char* err = nullptr;
    const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) {
            out_err = err;
            sqlite3_free(err);
        } else {
            out_err = sqlite_err(db_);
        }
        return false;
    }
    return true;
}

std::string DatabaseManager::generate_salt(size_t bytes) {
    std::string salt;
    salt.resize(bytes);

    if (RAND_bytes(reinterpret_cast<unsigned char*>(&salt[0]), (int)bytes) != 1) {
        // fallback simple (aunque no es criptográficamente perfecto)
        for (size_t i = 0; i < bytes; ++i) salt[i] = char('a' + (i % 26));
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : salt) {
        oss << std::setw(2) << (int)c;
    }
    return oss.str();
}

std::string DatabaseManager::sha256_hex(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.data()), input.size(), hash);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::setw(2) << (int)hash[i];
    }
    return oss.str();
}

std::string DatabaseManager::h_password(const std::string& password, const std::string& salt) {
    return sha256_hex(password + salt);
}

bool DatabaseManager::create_user(const std::string& username, const std::string& password, std::string& out_err) {
    if (username.empty()) {
        out_err = "username vacío";
        return false;
    }
    if (password.empty()) {
        out_err = "password vacía";
        return false;
    }

    std::string salt = generate_salt(16);
    std::string pass_hash = h_password(password, salt);

    const char* sql =
        "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pass_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        out_err = sqlite_err(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::optional<User> DatabaseManager::verify_user(const std::string& username, const std::string& password, std::string& out_err) {
    if (username.empty() || password.empty()) {
        out_err = "username o password vacío";
        return std::nullopt;
    }

    const char* sql =
        "SELECT id, password_hash, salt FROM users WHERE username = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        out_err = "usuario no encontrado";
        return std::nullopt;
    }

    int id = sqlite3_column_int(stmt, 0);
    const char* hash_c = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* salt_c = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

    std::string db_hash = hash_c ? hash_c : "";
    std::string db_salt = salt_c ? salt_c : "";

    sqlite3_finalize(stmt);

    if (h_password(password, db_salt) == db_hash) {
        return User(id, username, db_hash, db_salt);
    }

    out_err = "credenciales inválidas";
    return std::nullopt;
}

bool DatabaseManager::add_device(const Device& d, std::string& out_err) {
    const char* sql =
        "INSERT INTO devices (user_id, name, ip, type, status, interval_time) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return false;
    }

    sqlite3_bind_int(stmt, 1, d.user_id);
    sqlite3_bind_text(stmt, 2, d.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, d.ip.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, d.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, d.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, d.interval_time);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        out_err = sqlite_err(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::vector<Device> DatabaseManager::get_devices(int user_id, std::string& out_err) {
    std::vector<Device> results;

    const char* sql =
        "SELECT id, name, ip, type, status, interval_time "
        "FROM devices WHERE user_id = ? ORDER BY id ASC;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return results;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Device d;
        d.id = sqlite3_column_int(stmt, 0);
        d.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        d.ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        d.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        d.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        d.interval_time = sqlite3_column_int(stmt, 5);
        d.user_id = user_id;
        results.push_back(d);
    }

    sqlite3_finalize(stmt);
    return results;
}

bool DatabaseManager::delete_device(int id, int user_id, std::string& out_err) {
    const char* sql =
        "DELETE FROM devices WHERE id = ? AND user_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, user_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        out_err = sqlite_err(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool DatabaseManager::update_status(int device_id, const std::string& status, std::string& out_err) {
    const char* sql =
        "UPDATE devices SET status = ? WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return false;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, device_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        out_err = sqlite_err(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

static std::string current_datetime_utc_local() {
    // formato ISO-like compatible con SQLite y display: YYYY-MM-DD HH:MM:SS
    std::time_t t = std::time(nullptr);
    std::tm tm;
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

bool DatabaseManager::add_event(int device_id, const std::string& event, std::string& out_err) {
    const char* sql =
        "INSERT INTO events (device_id, event, datetime) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return false;
    }

    std::string dt = current_datetime_utc_local();

    sqlite3_bind_int(stmt, 1, device_id);
    sqlite3_bind_text(stmt, 2, event.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dt.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        out_err = sqlite_err(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::vector<Event> DatabaseManager::get_events(int device_id, std::string& out_err) {
    std::vector<Event> results;

    const char* sql =
        "SELECT id, event, datetime FROM events WHERE device_id = ? ORDER BY datetime ASC;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        out_err = sqlite_err(db_);
        return results;
    }

    sqlite3_bind_int(stmt, 1, device_id);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Event e;
        e.id = sqlite3_column_int(stmt, 0);
        e.event = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        e.datetime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        e.device_id = device_id;
        results.push_back(e);
    }

    sqlite3_finalize(stmt);
    return results;
}

