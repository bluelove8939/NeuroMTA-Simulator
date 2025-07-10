#ifndef __NEUROMTA_CONTEXT_H
#define __NEUROMTA_CONTEXT_H

#include <map>
#include <string>
// #include "core.h"

namespace neuromta {

class Core;
class Context{
private:
    int _timestamp = 0;
    std::map<std::string, Core *> _registered_cores;

public:
    void reset();
    void tick_clock();
    int  get_timestamp() const;
    void register_core(const std::string core_name, Core *core_p);
};

}

#endif