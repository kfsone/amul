/*
              ****    AMUL.INCS.H.....Adventure Compiler    ****
              ****              Include Files               ****
									   */

#include <stdio.h>			/* Standard I/O controls  */
#include <ctype.h>			/* Character type control */
#include <fcntl.h>			/* File control stuff	  */
#include <string.h>			/* String bits and pieces */
#include <proto/exec.h>			/* Exec protos..	  */
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <exec/types.h>			/* Exec bits and pieces   */
#include <exec/memory.h>		/* Memory details........ */
#include <exec/ports.h>			/* Message details	  */
#ifdef FRAME
#include <devices/serial.h>		/* Serial device info	  */
#include <intuition/intuition.h>	/* Eek! -MORE- Exec	  */
#endif
#include <ADV:H/AMUL.Stct.H>		/* AMUL Structures!	  */
