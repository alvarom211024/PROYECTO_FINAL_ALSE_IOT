// mainwindow.cpp

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
    , simuladorTimer_(nullptr)
{
    ui->setupUi(this);

    if (!db_.init("iot.db")) {
        QMessageBox::critical(
            this,
            "Error",
            "No se pudo abrir la base de datos."
            );
    }

    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(0);
    }

    simuladorTimer_ = new QTimer(this);

    connect(
        simuladorTimer_,
        &QTimer::timeout,
        this,
        &MainWindow::actualizarEstadoSimulado
        );

    simuladorTimer_->start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showError(const QString& msg)
{
    QMessageBox::warning(this, "Error", msg);
}

void MainWindow::showInfo(const QString& msg)
{
    QMessageBox::information(this, "Información", msg);
}

void MainWindow::refrescarListaDispositivos()
{
    if (!current_user_) return;

    ui->listWidget->clear();

    std::string err;

    lista_dispositivos_actuales_ =
        db_.get_devices(current_user_->id, err);

    for (const auto& disp : lista_dispositivos_actuales_) {

        QString itemText =
            QString("%1 | %2 | %3 | %4")
                .arg(QString::fromStdString(disp.name))
                .arg(QString::fromStdString(disp.type))
                .arg(QString::fromStdString(disp.ip))
                .arg(QString::fromStdString(disp.status));

        ui->listWidget->addItem(itemText);
    }
}

// PAGE 0

void MainWindow::on_pushButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_clicked()
{
    close();
}

// LOGIN

void MainWindow::on_pushButton_10_clicked()
{
    QString username =
        ui->lineEdit->text().trimmed();

    QString password =
        ui->lineEdit_2->text();

    if (username.isEmpty() || password.isEmpty()) {

        showError("Complete todos los campos.");
        return;
    }

    std::string err;

    if (db_.create_user(
            username.toStdString(),
            password.toStdString(),
            err
            ))
    {
        showInfo("Usuario registrado.");

        ui->lineEdit->clear();
        ui->lineEdit_2->clear();
    }
    else {

        showError(QString::fromStdString(err));
    }
}

void MainWindow::on_pushButton_16_clicked()
{
    QString username =
        ui->lineEdit->text().trimmed();

    QString password =
        ui->lineEdit_2->text();

    std::string err;

    auto user =
        db_.verify_user(
            username.toStdString(),
            password.toStdString(),
            err
            );

    if (user) {

        current_user_ = user;

        showInfo("Bienvenido " + username);

        ui->lineEdit->clear();
        ui->lineEdit_2->clear();

        refrescarListaDispositivos();

        ui->stackedWidget->setCurrentIndex(2);
    }
    else {

        showError("Credenciales incorrectas.");
    }
}

// PANEL

void MainWindow::on_pushButton_11_clicked()
{
    if (!current_user_) return;

    QString nombre =
        ui->lineEdit_3->text().trimmed();

    QString ip =
        ui->lineEdit_4->text().trimmed();

    QString tipo =
        ui->lineEdit_10->text().trimmed();

    int intervalo =
        ui->lineEdit_11->text().toInt();

    if (
        nombre.isEmpty() ||
        ip.isEmpty() ||
        tipo.isEmpty() ||
        intervalo <= 0
        )
    {
        showError("Complete todos los campos.");
        return;
    }

    Device d;

    d.user_id = current_user_->id;

    d.name = nombre.toStdString();

    d.ip = ip.toStdString();

    d.type = tipo.toStdString();

    d.status = "OFFLINE";

    d.interval_time = intervalo;

    std::string err;

    if (db_.add_device(d, err)) {

        showInfo("Dispositivo agregado.");

        refrescarListaDispositivos();

        ui->lineEdit_3->clear();
        ui->lineEdit_4->clear();
        ui->lineEdit_10->clear();
        ui->lineEdit_11->clear();
    }
    else {

        showError(QString::fromStdString(err));
    }
}

void MainWindow::on_pushButton_12_clicked()
{
    current_user_ = std::nullopt;

    ui->listWidget->clear();

    lista_dispositivos_actuales_.clear();

    ui->stackedWidget->setCurrentIndex(0);

    showInfo("Sesión cerrada.");
}

void MainWindow::on_pushButton_22_clicked()
{
    int fila =
        ui->listWidget->currentRow();

    if (
        fila < 0 ||
        fila >= static_cast<int>(
            lista_dispositivos_actuales_.size()
            )
        )
    {
        showError("Seleccione un dispositivo.");
        return;
    }

    auto disp =
        lista_dispositivos_actuales_[fila];

    std::string err;

    if (
        db_.delete_device(
            disp.id,
            current_user_->id,
            err
            )
        )
    {
        showInfo("Dispositivo eliminado.");

        refrescarListaDispositivos();
    }
    else {

        showError(QString::fromStdString(err));
    }
}

void MainWindow::on_pushButton_23_clicked()
{
    int fila =
        ui->listWidget->currentRow();

    if (
        fila < 0 ||
        fila >= static_cast<int>(
            lista_dispositivos_actuales_.size()
            )
        )
    {
        showError("Seleccione un dispositivo.");
        return;
    }

    auto disp =
        lista_dispositivos_actuales_[fila];

    id_dispositivo_seleccionado_ = disp.id;

    ui->listWidget_2->clear();

    std::string err;

    auto eventos =
        db_.get_events(
            disp.id,
            err
            );

    for (const auto& ev : eventos) {

        QString item =
            QString("[%1] %2")
                .arg(QString::fromStdString(ev.datetime))
                .arg(QString::fromStdString(ev.event));

        ui->listWidget_2->addItem(item);
    }

    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_pushButton_26_clicked()
{
    int fila =
        ui->listWidget->currentRow();

    if (
        fila < 0 ||
        fila >= static_cast<int>(
            lista_dispositivos_actuales_.size()
            )
        )
    {
        showError("Seleccione un dispositivo.");
        return;
    }

    Device disp =
        lista_dispositivos_actuales_[fila];

    QString nombre =
        ui->lineEdit_3->text().trimmed();

    QString ip =
        ui->lineEdit_4->text().trimmed();

    QString tipo =
        ui->lineEdit_10->text().trimmed();

    int intervalo =
        ui->lineEdit_11->text().toInt();

    sqlite3* db;

    sqlite3_open("iot.db", &db);

    const char* sql =
        "UPDATE devices "
        "SET name=?, ip=?, type=?, interval_time=? "
        "WHERE id=?;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        db,
        sql,
        -1,
        &stmt,
        nullptr
        );

    sqlite3_bind_text(
        stmt,
        1,
        nombre.toStdString().c_str(),
        -1,
        SQLITE_TRANSIENT
        );

    sqlite3_bind_text(
        stmt,
        2,
        ip.toStdString().c_str(),
        -1,
        SQLITE_TRANSIENT
        );

    sqlite3_bind_text(
        stmt,
        3,
        tipo.toStdString().c_str(),
        -1,
        SQLITE_TRANSIENT
        );

    sqlite3_bind_int(
        stmt,
        4,
        intervalo
        );

    sqlite3_bind_int(
        stmt,
        5,
        disp.id
        );

    int rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {

        showInfo("Dispositivo actualizado.");

        refrescarListaDispositivos();
    }
    else {

        showError("No se pudo editar.");
    }

    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

// HISTORIAL

void MainWindow::on_pushButton_24_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_pushButton_25_clicked()
{
    guardarHistorialCSV();
}

void MainWindow::guardarHistorialCSV()
{
    if (!id_dispositivo_seleccionado_) return;

    QString ruta =
        QFileDialog::getSaveFileName(
            this,
            "Guardar CSV",
            "",
            "CSV (*.csv)"
            );

    if (ruta.isEmpty()) return;

    QFile archivo(ruta);

    if (!archivo.open(
            QIODevice::WriteOnly |
            QIODevice::Text
            ))
    {
        showError("No se pudo guardar.");
        return;
    }

    QTextStream out(&archivo);

    out << "Fecha,Evento\n";

    std::string err;

    auto eventos =
        db_.get_events(
            *id_dispositivo_seleccionado_,
            err
            );

    for (const auto& ev : eventos) {

        out
            << QString::fromStdString(ev.datetime)
            << ","
            << QString::fromStdString(ev.event)
            << "\n";
    }

    archivo.close();

    showInfo("CSV exportado.");
}

// SIMULADOR

void MainWindow::actualizarEstadoSimulado()
{
    if (!current_user_) return;

    std::string err;

    for (auto& disp : lista_dispositivos_actuales_) {

        bool online =
            QRandomGenerator::global()->bounded(2);

        QString estado;

        if (online)
            estado = "ONLINE";
        else
            estado = "OFFLINE";

        if (disp.status != estado.toStdString()) {

            disp.status =
                estado.toStdString();

            db_.update_status(
                disp.id,
                disp.status,
                err
                );

            QString evento =
                "Cambio de estado a " + estado;

            db_.add_event(
                disp.id,
                evento.toStdString(),
                err
                );
        }
    }

    refrescarListaDispositivos();
}
