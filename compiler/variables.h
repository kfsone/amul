#pragma once
// Temporary file to track globals used by the compiler

extern int     dmoves;        // How many RF_CEMETERYs to check?
extern int     rmn;           // Current room no.
extern int32_t FPos;          // Used during TT/Lang writes
extern int     proc;          // What we are processing
extern char *  data;          // Pointer to data buffer
extern char *  data2;         // Secondary buffer area
extern char *  syntab;        // Synonym table, re-read
extern int32_t datal2, mins;  // Length of data! & gametime
extern int32_t obmem;         // Size of Objects.TXT
extern int32_t wizstr;        // Wizards strength
extern char *  mobdat;        // Mobile data
enum { WORD_LEN = 64 };
extern char Word[WORD_LEN];

extern char  fnm[150];
extern char  was[128];
extern FILE *ofp5;

extern struct _OBJ_STRUCT2 *obtab2, *objtab2, obj2, *osrch, *osrch2;
