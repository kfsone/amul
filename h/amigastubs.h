#pragma once
// Provide stubbing for various Amiga structures and types.

#if defined(AMIGA)
#	include <devices/timer.h>
#endif

#include <cinttypes>

namespace Amiga {

using UWORD = uint16_t;
using ULONG = uint32_t;
using WORD = int16_t;
using SHORT = int16_t;
using COUNT = int16_t;
using UBYTE = uint8_t;
using BYTE = int8_t;
using VOID = void;

struct Node
{
	Node *ln_Succ; // Pointer to next (successor) 
	Node *ln_Pred; // Pointer to previous (predecessor) 
	UBYTE ln_Type;
	BYTE  ln_Pri;  // Priority, for sorting 
	char *ln_Name; // ID string, null terminated 
};

// http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node007D.html
struct List
{
	Node *lh_Head;
	Node *lh_Tail;
	Node *lh_TailPred;
	UBYTE lh_Type;
	UBYTE l_pad;
};

struct Task
{
	Node  tc_Node;
	UBYTE tc_Flags;
	UBYTE tc_State;
	BYTE  tc_IDNestCnt;  // intr disabled nesting
	BYTE  tc_TDNestCnt;  // task disabled nesting
	ULONG tc_SigAlloc;   // sigs allocated 
	ULONG tc_SigWait;	// sigs we are waiting for 
	ULONG tc_SigRecvd;   // sigs we have received 
	ULONG tc_SigExcept;  // sigs we will take excepts for 
	UWORD tc_TrapAlloc;  // traps allocated 
	UWORD tc_TrapAble;   // traps enabled 
	void *tc_ExceptData; // points to except data 
	void *tc_ExceptCode; // points to except code 
	void *tc_TrapData;   // points to trap code 
	void *tc_TrapCode;   // points to trap data 
	void *tc_SPReg;		 // stack pointer	    
	void *tc_SPLower;	// stack lower bound    
	void *tc_SPUpper;	// stack upper bound + 2
	VOID (*tc_Switch)(); // task losing CPU	  
	VOID (*tc_Launch)(); // task getting CPU  
	List  tc_MemEntry;   // Allocated memory. Freed by RemTask() 
	void *tc_UserData;   // For use by the task; no restrictions! 
};

struct MsgPort
{
	Node  mp_Node;
	UBYTE mp_Flags;
	UBYTE mp_SigBit;  // signal bit number	
	void *mp_SigTask; // object to be signalled 
	List  mp_MsgList; // message linked list	
};

struct Message
{
	Node	 mn_Node;
	MsgPort *mn_ReplyPort; // message reply port 
	UWORD	mn_Length;	// total message length, in bytes 
						   // (include the size of the Message 
						   // structure in the length) 
};

struct Device
{};
struct Unit
{};

struct IORequest
{
	Message io_Message;
	Device *io_Device;  // device node pointer  
	Unit *  io_Unit;	// unit (driver private)
	UWORD   io_Command; // device command 
	UBYTE   io_Flags;
	BYTE	io_Error; // error or warning num 
};

struct IOStdReq
{
	Message io_Message;
	Device *io_Device;  // device node pointer  
	Unit *  io_Unit;	// unit (driver private)
	UWORD   io_Command; // device command 
	UBYTE   io_Flags;
	BYTE	io_Error;  // error or warning num 
	ULONG   io_Actual; // actual number of bytes transferred 
	ULONG   io_Length; // requested number bytes transferred
	void *  io_Data;   // points to data area 
	ULONG   io_Offset; // offset for block structured devices 
};

#define mp_SoftInt mp_SigTask // Alias 

// mp_Flags: Port arrival actions (PutMsg) 
#define PF_ACTION 3  // Mask 
#define PA_SIGNAL 0  // Signal task in mp_SigTask 
#define PA_SOFTINT 1 // Signal SoftInt in mp_SoftInt/mp_SigTask 
#define PA_IGNORE 2  // Ignore arrival 

// prevent/reallow scheduling
void Forbid();
void Permit();

struct ScheduleGuard final
{
	ScheduleGuard();
	~ScheduleGuard();
};

int32_t Wait(int32_t signalSet);

Task *   FindTask(const char *name);
MsgPort *FindPort(const char *portName);
MsgPort *CreatePort(const char *portName, uint32_t priority);
void	 DeletePort(MsgPort *);

void	 PutMsg(MsgPort *port, Message *msg);
Message *GetMsg(MsgPort *port);
void	 ReplyMsg(Message *msg);
Message *WaitPort(MsgPort *port);

// Wait this many 20ths of a second
void Delay(int ticks);

}  // namespace Amiga