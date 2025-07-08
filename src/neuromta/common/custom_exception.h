#ifndef __NEUROMTA_CUSTOM_EXCEPTION
#define __NEUROMTA_CUSTOM_EXCEPTION

#include <iostream>
#include <stdexcept>
#include <string>

namespace neuromta {

class NeuroMTAException: public std::exception {
protected:
    std::string message;
public:
    NeuroMTAException(std::string message): message(message) {}
    virtual const char * what () {
        std::cerr << "[ERROR] " << this->message << std::endl;
        return ("[ERROR] " + this->message).c_str();
    }
};


class NeuroMTADeviceException: public NeuroMTAException {
public:
    NeuroMTADeviceException(std::string module_name, std::string message): NeuroMTAException("device error: " + module_name + ": " + message) {}
};


class NeuroMTARuntimeException: public NeuroMTAException {
public:
    NeuroMTARuntimeException(std::string runtime_name, std::string message): NeuroMTAException("runtime error: " + runtime_name + ": " + message) {}
};

}

#endif
