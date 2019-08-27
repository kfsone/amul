#ifndef SRC_H_AMUL_H
#define SRC_H_AMUL_H

#include "h/amul.defs.h"

void launchWorker(WorkerType type, void *context);
void shutdownWorkers() noexcept;

#endif  // SRC_H_AMUL_H
