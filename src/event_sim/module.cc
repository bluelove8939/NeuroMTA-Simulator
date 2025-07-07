#include "module.h"

namespace event_sim {

Module::Module(): context_p(0) {}
Module::Module(Context *context_p): context_p(context_p) {}

}