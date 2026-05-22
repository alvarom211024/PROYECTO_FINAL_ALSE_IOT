#ifndef EVENT_H
#define EVENT_H

#include <string>

class Event {
public:
    int id{-1};
    int device_id{-1};

    std::string event;
    std::string datetime;
};

#endif
