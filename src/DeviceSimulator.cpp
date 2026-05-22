#include "DeviceSimulator.h"

DeviceSimulator::DeviceSimulator(QObject *parent)
    : QObject(parent),
    online(false)
{
    connect(
        &timer,
        &QTimer::timeout,
        this,
        &DeviceSimulator::toggleStatus
        );
}

void DeviceSimulator::start()
{
    timer.start(3000);
}

void DeviceSimulator::toggleStatus()
{
    online = !online;

    if(online)
        emit statusChanged("ONLINE");
    else
        emit statusChanged("OFFLINE");
}
