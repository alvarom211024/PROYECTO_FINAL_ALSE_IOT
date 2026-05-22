/**
 * @file DatabaseManager.h
 * @brief Gestión de base de datos SQLite del sistema IoT.
 */

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <sqlite3.h>
#include <optional>
#include <vector>
#include <string>

#include "User.h"
#include "Device.h"
#include "Event.h"

/**
 * @class DatabaseManager
 * @brief Clase encargada de administrar la base de datos.
 *
 * Permite:
 * - Crear usuarios
 * - Verificar autenticación
 * - Gestionar dispositivos
 * - Registrar eventos
 * - Consultar historial
 */
class DatabaseManager {
public:

    /**
     * @brief Constructor.
     */
    DatabaseManager();

    /**
     * @brief Destructor.
     */
    ~DatabaseManager();

    /**
     * @brief Inicializa la base de datos.
     * @param filename Nombre del archivo SQLite.
     * @return true si se inicializó correctamente.
     */
    bool init(const std::string& filename = "iot.db");

    /**
     * @brief Crea un usuario nuevo.
     * @param username Nombre de usuario.
     * @param password Contraseña.
     * @param out_err Mensaje de error.
     * @return true si el usuario fue creado.
     */
    bool create_user(
        const std::string& username,
        const std::string& password,
        std::string& out_err
        );

    /**
     * @brief Verifica credenciales de usuario.
     * @param username Usuario.
     * @param password Contraseña.
     * @param out_err Error generado.
     * @return Usuario autenticado.
     */
    std::optional<User> verify_user(
        const std::string& username,
        const std::string& password,
        std::string& out_err
        );

    /**
     * @brief Agrega un dispositivo.
     * @param d Dispositivo.
     * @param out_err Error generado.
     * @return true si fue agregado.
     */
    bool add_device(const Device& d, std::string& out_err);

    /**
     * @brief Obtiene dispositivos de un usuario.
     * @param user_id ID del usuario.
     * @param out_err Error generado.
     * @return Vector de dispositivos.
     */
    std::vector<Device> get_devices(
        int user_id,
        std::string& out_err
        );

    /**
     * @brief Elimina un dispositivo.
     * @param id ID del dispositivo.
     * @param user_id ID del usuario propietario.
     * @param out_err Error generado.
     * @return true si fue eliminado.
     */
    bool delete_device(
        int id,
        int user_id,
        std::string& out_err
        );

    /**
     * @brief Actualiza estado del dispositivo.
     * @param device_id ID dispositivo.
     * @param status Nuevo estado.
     * @param out_err Error generado.
     * @return true si se actualizó.
     */
    bool update_status(
        int device_id,
        const std::string& status,
        std::string& out_err
        );

    /**
     * @brief Agrega un evento al historial.
     * @param device_id ID dispositivo.
     * @param event Evento generado.
     * @param out_err Error generado.
     * @return true si fue agregado.
     */
    bool add_event(
        int device_id,
        const std::string& event,
        std::string& out_err
        );

    /**
     * @brief Obtiene historial de eventos.
     * @param device_id ID dispositivo.
     * @param out_err Error generado.
     * @return Lista de eventos.
     */
    std::vector<Event> get_events(
        int device_id,
        std::string& out_err
        );

private:

    /**
     * @brief Puntero de conexión SQLite.
     */
    sqlite3* db_;

    /**
     * @brief Ejecuta sentencias SQL simples.
     * @param sql Consulta SQL.
     * @param out_err Error generado.
     * @return true si se ejecutó correctamente.
     */
    bool execute_sql(
        const std::string& sql,
        std::string& out_err
        );

    /**
     * @brief Genera salt aleatorio.
     * @param bytes Cantidad de bytes.
     * @return Salt generado.
     */
    static std::string generate_salt(size_t bytes = 16);

    /**
     * @brief Genera hash SHA256.
     * @param input Texto entrada.
     * @return Hash generado.
     */
    static std::string sha256_hex(const std::string& input);

    /**
     * @brief Genera contraseña cifrada.
     * @param password Contraseña.
     * @param salt Salt.
     * @return Hash final.
     */
    static std::string h_password(
        const std::string& password,
        const std::string& salt
        );
};

#endif