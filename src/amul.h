#ifndef SRC_H_AMUL_H
#define SRC_H_AMUL_H

#include "amul.typedefs.h"
#include "workertype.h"

void StartClient(WorkerType type, slotid_t, void *context);
void KillClients() noexcept;

#endif  // SRC_H_AMUL_H
