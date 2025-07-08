#ifndef __NEUROMTA_CONTEXT_H
#define __NEUROMTA_CONTEXT_H

namespace neuromta {

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