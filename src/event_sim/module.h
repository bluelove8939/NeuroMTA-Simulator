#ifndef __EVENT_SIM_MODULE_H
#define __EVENT_SIM_MODULE_H

#include <string>

#include "context.h"
#include "command.h"

namespace event_sim {

class Module {
public:
    Context *context_p;
    std::string name;

    Module();
    Module(Context *context_p);
    virtual ~Module() {}

    virtual bool issue_command(Command *command_p) = 0;
    virtual void tick_clock() = 0;
};

}

#endif