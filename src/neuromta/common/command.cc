#include "command.h"


namespace neuromta {

Command::Command(const std::string action): action(action) {}

bool Command::is_issued() {
    return this->issue_time != -1;
}

bool Command::is_committed() {
    return this->commit_time != -1;
}

}
