// mainwindow.h

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    // PAGE 0
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();

    // LOGIN
    void on_pushButton_10_clicked();
    void on_pushButton_16_clicked();

    // PANEL
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_22_clicked();
    void on_pushButton_23_clicked();
    void on_pushButton_26_clicked();

    // HISTORIAL
    void on_pushButton_24_clicked();
    void on_pushButton_25_clicked();

    void actualizarEstadoSimulado();

private:
    Ui::MainWindow *ui;

    DatabaseManager db_;

    std::optional<User> current_user_;

    std::vector<Device> lista_dispositivos_actuales_;

    std::optional<int> id_dispositivo_seleccionado_;

    QTimer* simuladorTimer_;

    void showError(const QString& msg);

    void showInfo(const QString& msg);

    void refrescarListaDispositivos();

    void guardarHistorialCSV();
};

#endif // MAINWINDOW_H
