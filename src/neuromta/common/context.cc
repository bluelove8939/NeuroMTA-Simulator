#include "context.h"
#include "core.h"
#include "custom_exception.h"

namespace neuromta {

void Context::reset() {
    for (auto it = this->_registered_cores.begin(); it != this->_registered_cores.end(); it++)
        it->second->reset();
    this->_timestamp = 0;
}

void Context::tick_clock() {
    for (auto it = this->_registered_cores.begin(); it != this->_registered_cores.end(); it++)
        it->second->tick_clock();
    this->_timestamp++;
}

int Context::get_timestamp() const {
    return this->_timestamp;
}

void Context::register_core(const std::string core_name, Core *core_p) {    
    this->_registered_cores[core_name] = core_p;
}

}