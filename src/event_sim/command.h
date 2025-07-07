#ifndef __EVENT_SIM_COMMAND_H
#define __EVENT_SIM_COMMAND_H

#include <vector>
#include <string>

namespace event_sim {

typedef uint64_t cmd_arg_t;  // command argument type
typedef int32_t  slot_id_t;  // slot ID type

class Command {
public:
    const std::string       action;
    std::vector<cmd_arg_t>  args;
    slot_id_t               dst_dept=-1;
    std::vector<slot_id_t>  src_dept;
    slot_id_t               module_dept=-1;

    int issue_time  = -1;
    int commit_time = -1;

    Command(const std::string action);

    bool is_issued();
    bool is_committed();
};

}

#endif