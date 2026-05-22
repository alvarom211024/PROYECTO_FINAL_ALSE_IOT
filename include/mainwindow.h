#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
 * @file mainwindow.h
 * @brief Ventana principal del sistema IoT.
 */

#include <QWidget>
#include <QTimer>

#include <optional>
#include <vector>

#include "DatabaseManager.h"
#include "Device.h"
#include "Event.h"
#include "User.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief Clase principal de la interfaz gráfica.
 */
class MainWindow : public QWidget
{
    Q_OBJECT

public:

    /**
     * @brief Constructor principal.
     * @param parent Widget padre.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MainWindow();

private slots:

    /**
     * @brief Ir a login.
     */
    void on_pushButton_2_clicked();

    /**
     * @brief Salir de la aplicación.
     */
    void on_pushButton_clicked();

    /**
     * @brief Registrar usuario.
     */
    void on_pushButton_10_clicked();

    /**
     * @brief Iniciar sesión.
     */
    void on_pushButton_16_clicked();

    /**
     * @brief Agregar dispositivo.
     */
    void on_pushButton_11_clicked();

    /**
     * @brief Cerrar sesión.
     */
    void on_pushButton_12_clicked();

    /**
     * @brief Eliminar dispositivo.
     */
    void on_pushButton_22_clicked();

    /**
     * @brief Ver historial.
     */
    void on_pushButton_23_clicked();

    /**
     * @brief Editar dispositivo.
     */
    void on_pushButton_26_clicked();

    /**
     * @brief Volver al panel principal.
     */
    void on_pushButton_24_clicked();

    /**
     * @brief Exportar historial CSV.
     */
    void on_pushButton_25_clicked();

    /**
     * @brief Actualiza estados simulados.
     */
    void actualizarEstadoSimulado();

private:

    Ui::MainWindow *ui;

    DatabaseManager db_;

    std::optional<User> current_user_;

    std::optional<int> id_dispositivo_seleccionado_;

    std::vector<Device> lista_dispositivos_actuales_;

    QTimer* simuladorTimer_;

    /**
     * @brief Muestra mensajes de error.
     * @param msg Mensaje.
     */
    void showError(const QString& msg);

    /**
     * @brief Muestra mensajes informativos.
     * @param msg Mensaje.
     */
    void showInfo(const QString& msg);

    /**
     * @brief Refresca lista de dispositivos.
     */
    void refrescarListaDispositivos();

    /**
     * @brief Guarda historial CSV.
     */
    void guardarHistorialCSV();
};

#endif // MAINWINDOW_H
