#include "context.h"


namespace neuromta {

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