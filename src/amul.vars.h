#pragma once
#ifndef AMUL_VARS_H
#define AMUL_VARS_H

/*
 ****    AMUL.VARS.H.....Adventure Compiler    ****
 ****           Internal Variables!            ****
 */

// Manager Port: This is the port everyone will use to talk to the primary game server thread.
// TBH, I think it should either be a shared_ptr or it should a global. At the very least, it
// should be ref counted so that the manager doesn't accidentally delete it before everyone
// else has finished using it.
#include <memory>

extern thread_local std::shared_ptr<struct MsgPort> t_managerPort;

extern struct NPCClass npc, *npcp;

extern bool g_quiet;

#endif
