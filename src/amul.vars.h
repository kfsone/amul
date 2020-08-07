#pragma once
#ifndef AMUL_VARS_H
#define AMUL_VARS_H

#include <atomic>
#include "amul.typedefs.h"

extern thread_local std::atomic_bool t_terminate;

extern thread_local shared_ptr<struct MsgPort> t_managerPort;

extern bool g_quiet;

#endif

