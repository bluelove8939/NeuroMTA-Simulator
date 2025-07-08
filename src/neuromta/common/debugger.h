#ifndef __NEUROMTA_DEBUGGER_H
#define __NEUROMTA_DEBUGGER_H

#include <iostream>

#include "context.h"
#include "module.h"
#include "command.h"

namespace neuromta {

class Debugger {
    public:
        Context *context_p;
    
        Debugger(Context *context_p=NULL);
        virtual ~Debugger();
        virtual void print_log(const Module *module_p, const Command *command_p) const;
    };

}

#endif