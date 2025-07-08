#include "core.h"
#include "custom_exception.h"


namespace neuromta {

Core::Core(const int command_queue_depth): _command_queue_depth(command_queue_depth) {}
Core::~Core() {}

void Core::register_module(const std::string name, Module *module_p) {
    if (this->_modules.find(name) != this->_modules.end())
        throw NeuroMTADeviceException("Core", "invalid name detected");

    this->_modules[name] = module_p;
    module_p->name = name;
    module_p->context_p = &(this->context);

    this->_module_id_mappings[name] = this->_modules.size();
}

void Core::register_debugger(Debugger *debugger_p) {
    this->_debugger_p = debugger_p;
    debugger_p->context_p = &(this->context);
}

Module* Core::get_module(const std::string name) {
    return this->_modules[name];
}

slot_id_t Core::get_module_id(const std::string name) {
    return this->_module_id_mappings[name];
}

bool Core::is_idle() {
    return this->_command_queue.size() == 0;
}

void Core::reset_context() {
    this->context.reset();
}

void Core::submit_command(std::string module_name, Command &command) {
    Module  *module_p  = this->get_module(module_name);

    unsigned long int queue_full_iter_cnt = 0;
    while (this->_command_queue.size() >= (long unsigned int)this->_command_queue_depth){
        this->tick_clock();
        queue_full_iter_cnt++;

        if (queue_full_iter_cnt >= this->_single_command_cycle_thres) {
            std::cerr << "[WARNING] The command is submitted even though the command queue is full right now. "
                      << "It may cause the undeterministic operation of the device. "
                      << "Please set the 'single_command_cycle_thres' option larger than " << this->_single_command_cycle_thres
                      << " to eliminate this warning" << std::endl;
            std::cout << "COMMAND QUEUE" << std::endl;
            this->print_command_queue();
            throw NeuroMTADeviceException("Core", "command submission failed with deadlock");
        }
    }

    this->_command_queue.push_back(std::pair<Module*, Command>(module_p, command));
}

void Core::set_single_command_cycle_thres(const unsigned long int single_command_cycle_thres) {
    this->_single_command_cycle_thres = single_command_cycle_thres;
}

void Core::set_synchronization_cycle_thres(const unsigned long int synchronization_cycle_thres) {
    this->_synchronization_cycle_thres = synchronization_cycle_thres;
}

void Core::print_command_queue() {
    int i=0;
    
    for (auto entry: this->_command_queue) {
        std::cout << "entry:\t" << i << " \t" << entry.first->name << "." << entry.second.action << " "
                  << "\tdst_dept=" << entry.second.dst_dept << " "
                  << "\tsrc_dept=(";
        for (auto src_dept: entry.second.src_dept) 
            std::cout << " " << src_dept;
        std::cout << " ) \targs=(";
        for (auto arg: entry.second.args) 
            std::cout << " " << arg;
        std::cout << " )" << std::endl; 
        
        i++;
    }
}

void Core::tick_clock() {
    if (this->_command_queue.size()) {
        Module  *module_p  = this->_command_queue[0].first;
        Command *command_p = &(this->_command_queue[0].second);

        if (!command_p->is_issued()) {
            if (module_p->issue_command(command_p)) {
                command_p->issue_time = this->context.get_timestamp();
            }
        }

        if (command_p->is_committed()) {
            if (this->_debugger_p)
                this->_debugger_p->print_log(module_p, command_p);
            
            this->_command_queue.pop_front();
        }
    }

    this->context.tick_clock();
    for (auto cursor: this->_modules)
        cursor.second->tick_clock();
}

void Core::synchronize() {
    long unsigned int tick_iter_cnt = 0;
    
    while (!this->is_idle()) {
        this->tick_clock();
        tick_iter_cnt++;
        
        if (tick_iter_cnt >= this->_synchronization_cycle_thres) {
            std::cerr << "[WARNING] The synchronization process is pre-terminated because of the predefined option. "
                      << "Please set the 'synchronization_cycle_thres' option larger than " << this->_synchronization_cycle_thres
                      << " to eliminate this warning. "
                      << "If it is guaranteed that the synchronization does not take more cycles than the predefined option, "
                      << "there must be a deadlock caused by the control conflict." << std::endl;
            std::cout << "COMMAND QUEUE" << std::endl;
            this->print_command_queue();
            throw NeuroMTADeviceException("Core", "synchronization failed with deadlock");
        }
    }
}


CoreO3::CoreO3(
    const int command_queue_depth, 
    const int command_queue_window_limit
): Core(command_queue_depth), 
   _command_queue_window_limit(command_queue_window_limit) {}

void CoreO3::reset_context() {
    this->context.reset();
    this->_command_queue_window = 1;
}

void CoreO3::tick_clock() {
    const int window_end    = std::min<int>(this->_command_queue_window, this->_command_queue.size());

    std::set<slot_id_t> dst_dept_slots;
    std::set<slot_id_t> src_dept_slots;
    std::set<slot_id_t> module_dept_slots;

    auto cmd_q_st_it = this->_command_queue.begin();
    auto cmd_q_ed_it = this->_command_queue.begin() + window_end;

    for (auto cursor = cmd_q_st_it; cursor != cmd_q_ed_it; cursor++) {
        Module  *module_p  = cursor->first;
        Command *command_p = &(cursor->second);

        // Check dependency flag
        bool dept_flag = true;

        dept_flag = dept_flag && (src_dept_slots.find(command_p->dst_dept) == src_dept_slots.end());
        for (auto src_dept: command_p->src_dept) {
            dept_flag = dept_flag && (dst_dept_slots.find(src_dept) == dst_dept_slots.end());
        }
        dept_flag = dept_flag && (module_dept_slots.find(command_p->module_dept) == module_dept_slots.end());
        
        // Issue command (if necessary)
        if (!command_p->is_issued() && dept_flag) {
            if (module_p->issue_command(command_p))
                command_p->issue_time = this->context.get_timestamp();
        }

        // Update dependency slots
        if (!command_p->is_committed()) {
            if (command_p->dst_dept != -1) 
                dst_dept_slots.insert(command_p->dst_dept);

            for (auto src_dept: command_p->src_dept) {
                if (src_dept != -1) 
                    src_dept_slots.insert(src_dept);
            }

            if (command_p->module_dept != -1)
                module_dept_slots.insert(command_p->module_dept);
        }
    }

    this->context.tick_clock();
    for (auto cursor: this->_modules)
        cursor.second->tick_clock();
    
    this->_command_queue_window = std::min<int>(this->_command_queue_window + 1, this->_command_queue_window_limit);

    while (this->_command_queue.size()) {
        if (this->_command_queue[0].second.is_committed()) {
            if (this->_debugger_p)
                this->_debugger_p->print_log(this->_command_queue[0].first, &(this->_command_queue[0].second));

            // delete this->_command_queue[0].second;
            this->_command_queue.pop_front();
        } else {
            break;
        }
    }
}

}  // end of namespace: event_sim