#include "saos.h"

namespace event_sim {
namespace saos {

GEMMCoreSAOS::GEMMCoreSAOS(const int pe_height, const int pe_width)
: pe_height(pe_height), pe_width(pe_width) {
    this->_request_cnt = 0;
    this->_current_command = NULL;
}

bool GEMMCoreSAOS::issue_command(Command *command_p) {
    if (this->_request_cnt == 0 && this->_current_command == NULL) {
        if (command_p->action == "preload" || command_p->action == "flush")
            this->_request_cnt += this->pe_width;  // preload/flush with horizontal direction
        else if (command_p->action == "execute")
            this->_request_cnt += command_p->args[0];  // first argument should be the sequence length
        else 
            throw std::invalid_argument("Invalid argument");

        this->_current_command = command_p;

        return true;
    }
    
    return false;
}

void GEMMCoreSAOS::tick_clock() {
    if (this->_request_cnt)
        this->_request_cnt--;

    if (this->_request_cnt == 0 && this->_current_command != NULL) {
        this->_current_command->commit_time = this->context_p->get_timestamp();
        this->_current_command = NULL;
    }
}

}  // end of namespace: saos
}  // end of namespace: event_sim