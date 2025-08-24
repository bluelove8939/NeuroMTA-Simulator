#ifndef __NEUROMTA_ICNT_MODULE_H
#define __NEUROMTA_ICNT_MODULE_H

#include <queue>
#include "common/common.h"

namespace neuromta {

class InterconnectModuleBase: public Module {
private:
    void *_tfm_if_vp; // pointer to the TFM IF

    std::queue<Command*>    _suspended_commands;
    
    int                 _node_num;
    long unsigned int   _command_queue_depth;

public:
    InterconnectModuleBase(
        const std::string& config_file,
        const long unsigned int command_queue_depth
    );
    ~InterconnectModuleBase();

    virtual bool issue_command(Command *command_p)  override;
    virtual void cycle_step()                       override;
};

}

#endif