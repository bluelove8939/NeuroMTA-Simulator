#ifndef __EVENT_SIM_SAOS_H
#define __EVENT_SIM_SAOS_H

#include <stdexcept>

#include "event_sim.h"

namespace event_sim {
namespace saos {

class GEMMCoreSAOS: public Module {
public:
    const int pe_height;
    const int pe_width;

private:
    int _request_cnt;
    Command *_current_command; 

public:
    GEMMCoreSAOS(const int pe_height, const int pe_width);

    virtual bool issue_command(Command *command_p)  override;
    virtual void tick_clock()                       override;
};

}  // end of namespace: saos
}  // end of namespace: event_sim

#endif