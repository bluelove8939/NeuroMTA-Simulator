#ifndef __NEUROMTA_SAOS_MODULE_H
#define __NEUROMTA_SAOS_MODULE_H

#include "common/common.h"

namespace neuromta {

class SystolicArrayOS: public Module {
public:
    const int pe_height;
    const int pe_width;

private:
    int _request_cnt;
    Command *_current_command; 

public:
    SystolicArrayOS(const int pe_height, const int pe_width);

    virtual bool issue_command(Command *command_p)  override;
    virtual void tick_clock()                       override;
};

}

#endif