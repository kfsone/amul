#ifndef H_AMUL_INCS_H
#define H_AMUL_INCS_H 1
/*
 ****    AMUL.INCS.H.....Adventure Compiler    ****
 ****              Include Files               ****
 */

#include <ctype.h> /* Character type control */
#include <fcntl.h> /* File control stuff	  */
#include <stdbool.h>
#include <stdio.h>  /* Standard I/O controls  */
#include <string.h> /* String bits and pieces */
#if !defined(_MSC_VER)
#    include <unistd.h>
#endif
#if defined(__AMIGA__)
#    include <exec/memory.h> /* Memory details........ */
#    include <exec/ports.h>  /* Message details	  */
#    include <exec/types.h>  /* Exec bits and pieces   */
#    include <proto/dos.h>
#    include <proto/exec.h> /* Exec protos..	  */
#    include <proto/graphics.h>
#    include <proto/intuition.h>
#    ifdef FRAME
#        include <devices/serial.h>      /* Serial device info	  */
#        include <intuition/intuition.h> /* Eek! -MORE- Exec	  */
#    endif
#else
#    include "h/amigastubs.h"
#endif

#include "h/amul.stct.h" /* AMUL Structures!	  */

#endif
