#include "debugger.h"

namespace neuromta {

Debugger::Debugger(Context *context_p): context_p(context_p) {}
Debugger::~Debugger() {}

void Debugger::print_log(const Module *module_p, const Command *command_p) const {
    std::cout << "[DEBUG] #" << context_p->get_timestamp() << 
                 " \tmodule: " << module_p->name <<
                 " \taction: " << command_p->action << "\targs=(";

    for (auto arg: command_p->args) {
        std::cout << " " << arg;
    }

    std::cout << " )\tdst_dept=" << command_p->dst_dept << "\tsrc_dept=(";

    for (auto src_dept: command_p->src_dept) {
        std::cout << " " << src_dept;
    }

    std::cout << " )" <<
                 " \t( " << command_p->issue_time <<
                 " - " << command_p->commit_time << " )" << 
                 std::endl;
}

}