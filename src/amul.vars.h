#pragma once
#ifndef AMUL_VARS_H
#define AMUL_VARS_H

// Manager Port: This is the port everyone will use to talk to the primary game server thread.
// TBH, I think it should either be a shared_ptr or it should a global. At the very least, it
// should be ref counted so that the manager doesn't accidentally delete it before everyone
// else has finished using it.
#include <typedefs.h>

extern thread_local shared_ptr<struct MsgPort> t_managerPort;

extern bool g_quiet;

#endif
