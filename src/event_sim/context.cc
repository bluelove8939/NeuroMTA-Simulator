#include "context.h"


namespace event_sim {

void Context::reset() {
    this->_timestamp = 0;
}

void Context::tick_clock() {
    this->_timestamp++;
}

int Context::get_timestamp() const {
    return this->_timestamp;
}

}