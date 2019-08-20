#pragma once

#include "typedefs.h"

namespace Action::Demon
{

void Cancel(verbid_t verbId);
void ForceExecute(verbid_t verbId);
void Schedule(verbid_t verbId, int delay, bool global);
int Status(verbid_t verbId);

}  // namespace Action::Demon