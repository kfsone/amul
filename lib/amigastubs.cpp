#include "h/amul.incs.h"

#include <chrono>
#include <mutex>
#include <thread>

namespace Amiga
{
static Task myTask;

// For the sake of putting something roughly approximating Amiga's "Forbid" and
// "Permit", we use a simple mutex. This should ultimately go away.
static std::mutex scheduleMutex;

Task *
FindTask(const char *name)
{
    // Look for a task called "name", unless name is null, in which case we're
    // looking for our own task.
    if (name == nullptr) {
        return &myTask;
    } else {
        return nullptr;
    }
}

int32_t
Wait(int32_t signalSet)
{
    std::this_thread::yield();
    return -1;
}

MsgPort *
FindPort(const char *portName)
{
    return nullptr;
}

MsgPort *
CreatePort(const char *portName, uint32_t priority)
{
    return nullptr;
}

void
DeletePort(MsgPort *port)
{
}

void
Delay(int ticks)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20 * ticks));
}

void
Forbid()
{
    scheduleMutex.lock();
}

void
Permit()
{
    scheduleMutex.unlock();
}

ScheduleGuard::ScheduleGuard() { Forbid(); }

ScheduleGuard::~ScheduleGuard() { Permit(); }

}  // namespace Amiga