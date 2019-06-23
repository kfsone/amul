/*
 ****    AMUL.INCS.H.....Adventure Compiler    ****
 ****              Include Files               ****
 */

#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <cstdint>

#if defined(AMIGA)
#	include <exec/memory.h> /* Memory details........ */
#	include <exec/ports.h>  /* Message details	  */
#	include <exec/types.h>  /* Exec bits and pieces   */
#	include <proto/dos.h>
#	include <proto/exec.h> /* Exec protos..	  */
#else
#	include "h/amigastubs.h"
#endif

#include "h/os.h"

#include "h/amul.stct.h" /* AMUL Structures	  */
