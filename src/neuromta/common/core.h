#ifndef __NEUROMTA_CORE_H
#define __NEUROMTA_CORE_H

#include <map>
#include <deque>
#include <set>

#include "context.h"
#include "command.h"
#include "module.h"
#include "debugger.h"

namespace neuromta {

class Core {
protected:
    std::map<std::string, Module*> _modules; 
    std::map<std::string, slot_id_t> _module_id_mappings;

    std::deque<std::pair<Module*, Command>> _command_queue;
    const int _command_queue_depth;

    // This option is used when the client attempts to submit command to the device but the command queue is full 
    // The device will automatically ticks the clock until there is an empty command queue entry
    // If the iteration meets a limit, the device will print out warning message and submit the command in force
    unsigned long int _single_command_cycle_thres = 100000;
    unsigned long int _synchronization_cycle_thres   = 100000000;

    Debugger *_debugger_p = NULL;

    std::string _core_name = "__single_core";

public:
    Context *context_p;

public:
    Core(const int command_queue_depth=128);
    virtual ~Core();

    void        register_context(const std::string name, Context *context_p);
    void        register_module(const std::string name, Module *module_p);
    void        register_debugger(Debugger *debugger_p);
    Module*     get_module(const std::string name);
    slot_id_t   get_module_id(const std::string name);
    bool        is_idle();
    void        submit_command(std::string module_name, Command &command);

    void        set_single_command_cycle_thres(const unsigned long int single_command_cycle_thres);
    void        set_synchronization_cycle_thres  (const unsigned long int synchronization_cycle_thres  );
    
    void        print_command_queue();
    
    virtual void reset();
    virtual void cycle_step();
    void         synchronize();
};


class CoreO3: public Core {
private:
    const int _command_queue_window_limit; 
    int _command_queue_window = 1;

public:
    CoreO3(const int command_queue_depth, const int command_queue_window_limit);

    virtual void reset()        override;
    virtual void cycle_step()   override;
};

}

#endif