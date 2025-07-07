#ifndef __EVENT_SIM_DEBUGGER_H
#define __EVENT_SIM_DEBUGGER_H

#include <iostream>

#include "context.h"
#include "module.h"
#include "command.h"

namespace event_sim {

class Debugger {
    public:
        Context *context_p;
    
        Debugger(Context *context_p=NULL);
        virtual ~Debugger();
        virtual void print_log(const Module *module_p, const Command *command_p) const;
    };

}

#endif