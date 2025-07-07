#ifndef __EVENT_SIM_CONTEXT_H
#define __EVENT_SIM_CONTEXT_H

namespace event_sim {

class Context{
private:
    int _timestamp = 0;

public:
    void reset();
    void tick_clock();
    int  get_timestamp() const;
};

}

#endif