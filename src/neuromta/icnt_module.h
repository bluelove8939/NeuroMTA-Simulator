#ifndef __NEUROMTA_ICNT_MODULE_H
#define __NEUROMTA_ICNT_MODULE_H

#include "common/common.h"

namespace neuromta {

class InterconnectModule: public Module {


public:
    InterconnectModule();

    virtual bool issue_command(Command *command_p)  override;
    virtual void tick_clock()                       override;
};

}

#endif