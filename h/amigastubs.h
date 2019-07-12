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
#    endif

typedef uint16_t UWORD;
typedef uint32_t ULONG;
typedef int16_t  WORD;
typedef int16_t  SHORT;
typedef uint16_t USHORT;
typedef int16_t  COUNT;
typedef uint8_t  UBYTE;
typedef int8_t   BYTE;
typedef void     VOID;
typedef void *   APTR;

typedef struct Node {
    struct Node *ln_Succ;  // Pointer to next (successor)
    struct Node *ln_Pred;  // Pointer to previous (predecessor)
    UBYTE        ln_Type;
    BYTE         ln_Pri;   // Priority, for sorting
    char *       ln_Name;  // ID string, null terminated
} Node;

// http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node007D.html
typedef struct List {
    Node *lh_Head;
    Node *lh_Tail;
    Node *lh_TailPred;
    UBYTE lh_Type;
    UBYTE l_pad;
} List;

typedef struct Task {
    Node  tc_Node;
    UBYTE tc_Flags;
    UBYTE tc_State;
    BYTE  tc_IDNestCnt;   // intr disabled nesting
    BYTE  tc_TDNestCnt;   // task disabled nesting
    ULONG tc_SigAlloc;    // sigs allocated
    ULONG tc_SigWait;     // sigs we are waiting for
    ULONG tc_SigRecvd;    // sigs we have received
    ULONG tc_SigExcept;   // sigs we will take excepts for
    UWORD tc_TrapAlloc;   // traps allocated
    UWORD tc_TrapAble;    // traps enabled
    void *tc_ExceptData;  // points to except data
    void *tc_ExceptCode;  // points to except code
    void *tc_TrapData;    // points to trap code
    void *tc_TrapCode;    // points to trap data
    void *tc_SPReg;       // stack pointer
    void *tc_SPLower;     // stack lower bound
    void *tc_SPUpper;     // stack upper bound + 2
    VOID (*tc_Switch)();  // task losing CPU
    VOID (*tc_Launch)();  // task getting CPU
    List  tc_MemEntry;    // Allocated memory. Freed by RemTask()
    void *tc_UserData;    // For use by the task; no restrictions!
} Task;

typedef struct MsgPort {
    Node        mp_Node;
    UBYTE       mp_Flags;
    UBYTE       mp_SigBit;   // signal bit number
    void *      mp_SigTask;  // object to be signalled
    struct List mp_MsgList;  // message linked list
} MsgPort;

typedef struct Message {
    Node     mn_Node;
    MsgPort *mn_ReplyPort;  // message reply port
    UWORD    mn_Length;     // total message length, in bytes
                            // (include the size of the Message
                            // structure in the length)
} Message;

typedef struct Device {
    int i;
} Device;
typedef struct Unit {
    int i;
} Unit;

typedef struct IORequest {
    Message io_Message;
    Device *io_Device;   // device node pointer
    Unit *  io_Unit;     // unit (driver private)
    UWORD   io_Command;  // device command
    UBYTE   io_Flags;
    BYTE    io_Error;  // error or warning num
} IORequest;

typedef struct IOStdReq {
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
} IOStdReq;

#    define mp_SoftInt mp_SigTask  // Alias

// mp_Flags: Port arrival actions (PutMsg)
#    define PF_ACTION 3            // Mask
#    define PA_SIGNAL 0            // Signal task in mp_SigTask
#    define PA_SOFTINT 1           // Signal SoftInt in mp_SoftInt/mp_SigTask
#    define PA_IGNORE 2            // Ignore arrival

// prevent/reallow scheduling
void Forbid();
void Permit();

int32_t Wait(int32_t signalSet);

Task *   FindTask(const char *name);
MsgPort *FindPort(const char *portName);
MsgPort *CreatePort(const char *portName, uint32_t priority);
void     DeletePort(MsgPort *);

void     PutMsg(MsgPort *port, Message *msg);
Message *GetMsg(MsgPort *port);
void     ReplyMsg(Message *msg);
Message *WaitPort(MsgPort *port);

#    define MEMF_PUBLIC 0
static inline void *
AllocMem(size_t bytes, int flags)
{
    (void)flags;
    return calloc(bytes, 1);
}

static inline void
FreeMem(void *ptr, int size)
{
    free(ptr);
}

// Wait this many 20ths of a second
void Delay(int ticks);
#endif

#endif
