#include <chrono>
#include <thread>

#include "system.h"

// OS/portability functions

// Emulate the Amiga's 50hz time
using Tick = std::chrono::duration<int64_t, std::ratio<1, 50>>;

void
Delay(unsigned int ticks)
{
    // Amiga 'ticks' were 1/50th of a second.
    std::this_thread::sleep_for(Tick(ticks));
}

void
YieldCpu() noexcept
{
    std::this_thread::yield();
}

