#ifndef __EVENTSIM_CUSTOM_EXCEPTION
#define __EVENTSIM_CUSTOM_EXCEPTION

#include <iostream>
#include <stdexcept>
#include <string>

namespace event_sim {

class EventSimException: public std::exception {
protected:
    std::string message;
public:
    EventSimException(std::string message): message(message) {}
    virtual const char * what () {
        std::cerr << "[ERROR] " << this->message << std::endl;
        return ("[ERROR] " + this->message).c_str();
    }
};


class EventSimDeviceException: public EventSimException {
public:
    EventSimDeviceException(std::string module_name, std::string message): EventSimException("device error: " + module_name + ": " + message) {}
};


class EventSimRuntimeException: public EventSimException {
public:
    EventSimRuntimeException(std::string runtime_name, std::string message): EventSimException("runtime error: " + runtime_name + ": " + message) {}
};

}

#endif
