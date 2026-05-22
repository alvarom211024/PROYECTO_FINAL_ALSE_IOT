#ifndef DEVICE_SIMULATOR_H
#define DEVICE_SIMULATOR_H

#include <QObject>
#include <QTimer>

class DeviceSimulator : public QObject
{
    Q_OBJECT

public:
    explicit DeviceSimulator(QObject *parent = nullptr);

    void start();

signals:
    void statusChanged(QString status);

private slots:
    void toggleStatus();

private:
    QTimer timer;
    bool online;
};

#endif
