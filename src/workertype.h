#pragma once
#ifndef AMUL_WORKERTYPE_H
#define AMUL_WORKERTYPE_H

enum WorkerType {
    am_USER,  // User client
    am_DAEM,  // Demon handler
    am_MOBS,  // NPC handler
};
constexpr auto NUM_WORKER_TYPES = int(am_MOBS + 1);

#endif  // AMUL_WORKERTYPE_H

