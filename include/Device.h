#ifndef DEVICE_H
#define DEVICE_H

#include <string>

class Device {
public:
    int id{-1};
    int user_id{-1};

    std::string name;
    std::string ip;
    std::string type;
    std::string status;

    int interval_time{5};

    Device() = default;
};

#endif
