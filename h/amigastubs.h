#ifndef H_AMIGASTUBS_H
#define H_AMIGASTUBS_H 1

// Provide stubbing for various Amiga structures and types.

#if defined(AMIGA)
#    include <devices/timer.h>
#else

#    include <stdint.h>
#    include <stdlib.h>
#    include <string.h>

#    if !defined(_MSC_VER)
#        include <strings.h>
#        define stricmp strcasecmp
#        define strnicmp strncasecmp
#    endif

using UWORD = uint16_t;
using ULONG = uint32_t;
using WORD = int16_t;
using SHORT = int16_t;
using USHORT = uint16_t;
using COUNT = int16_t;
using UBYTE = uint8_t;
using BYTE = int8_t;
using VOID = void;
using APTR = void *;

#    define NT_MESSAGE 0
#    define NT_TASK 1
#    define NT_PORT 2

struct Node {
    constexpr Node(UBYTE type, const char *name = nullptr, BYTE pri = 0)
        : ln_Type(type)
        , ln_Name(name)
        , ln_Pri(pri)
    {
    }

    UBYTE       ln_Type;
    const char *ln_Name{nullptr};  // ID string, null terminated
    BYTE        ln_Pri{0};         // Priority, for sorting
};

struct Task : public Node {
    Task(const char *name = nullptr)
        : Node(NT_TASK, name)
    {
    }

    ULONG tc_SigAlloc;   // sigs allocated
    ULONG tc_SigWait;    // sigs we are waiting for
    ULONG tc_SigRecvd;   // sigs we have received
    ULONG tc_SigExcept;  // sigs we will take excepts for

    void *tc_UserData;  // For use by the task; no restrictions!
};

struct Device {
    int i;
};
struct Unit {
    int i;
};

#    ifdef NEVER
struct IORequest {
    Message io_Message;
    Device *io_Device;   // device node pointer
    Unit *  io_Unit;     // unit (driver private)
    UWORD   io_Command;  // device command
    UBYTE   io_Flags;
    BYTE    io_Error;  // error or warning num
};

struct IOStdReq {
    Message io_Message;
    Device *io_Device;   // device node pointer
    Unit *  io_Unit;     // unit (driver private)
    UWORD   io_Command;  // device command
    UBYTE   io_Flags;
    BYTE    io_Error;   // error or warning num
    ULONG   io_Actual;  // actual number of bytes transferred
    ULONG   io_Length;  // requested number bytes transferred
    void *  io_Data;    // points to data area
    ULONG   io_Offset;  // offset for block structured devices
};
#    endif

// prevent/reallow scheduling
void Forbid();
void Permit();

int32_t Wait(int32_t signalSet);

Task *FindTask(const char *name);

// Wait this many 20ths of a second
void Delay(unsigned int ticks);
#endif

#endif
