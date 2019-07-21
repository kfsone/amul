//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: h/Incs.H		Standard Includes
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

#include <stdio.h>			// Standard I/O controls
#include <ctype.h>			// Character type control
#include <fcntl.h>			// File control stuff
#include <string.h>			// String bits and pieces
#include <proto/exec.h>			// Exec protos.
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <amlib:proto/AGL.h>
#include <exec/types.h>			// Exec bits and pieces
#include <exec/memory.h>		// Memory details
#include <exec/ports.h>			// Message details
#ifdef FRAME
#include <devices/serial.h>		// Serial device info
#include <intuition/intuition.h>	// Eek! -MORE- Exec
#endif
#include <ADV:H/Stct.H>			// AGL Structures!
#include <AMLIB:Libraries/AGL.H>
#include <AMLIB:Proto/AGL.H>
