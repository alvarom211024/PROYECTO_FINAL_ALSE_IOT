/**
 * @file User.h
 * @brief Modelo de usuario del sistema.
 */

#ifndef USER_H
#define USER_H

#include <string>

/**
 * @class User
 * @brief Representa un usuario autenticado.
 */
class User {
public:

    int id{-1};

    std::string username;
    std::string password;
    std::string salt;

    /**
     * @brief Constructor vacío.
     */
    User();

    /**
     * @brief Constructor parametrizado.
     * @param id_ ID usuario.
     * @param user Nombre usuario.
     * @param pass Contraseña cifrada.
     * @param salt_ Salt de seguridad.
     */
    User(
        int id_,
        const std::string& user,
        const std::string& pass,
        const std::string& salt_
        );
};

#endif