/**
 * @file DeviceSimulator.h
 * @brief Simulador de conectividad IoT.
 */

#ifndef DEVICE_SIMULATOR_H
#define DEVICE_SIMULATOR_H

#include <QObject>
#include <QTimer>

/**
 * @class DeviceSimulator
 * @brief Simula cambios ONLINE/OFFLINE.
 */
class DeviceSimulator : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief Constructor.
     * @param parent Objeto padre.
     */
    explicit DeviceSimulator(QObject *parent = nullptr);

    /**
     * @brief Inicia simulación.
     */
    void start();

signals:

    /**
     * @brief Señal emitida cuando cambia el estado.
     * @param status Estado actual.
     */
    void statusChanged(QString status);

private slots:

    /**
     * @brief Alterna ONLINE/OFFLINE.
     */
    void toggleStatus();

private:

    QTimer timer;
    bool online;
};

#endif