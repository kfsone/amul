#ifndef AMUL_H_AMULINC_H
#define AMUL_H_AMULINC_H

#include "h/amul.defs.h"

extern struct _PLAYER *    usr, *me, him, *you;
extern struct LS *         linestat, *me2, *you2;
extern struct _VERB_STRUCT verb, *vbtab, *vbptr;
extern struct _SLOTTAB     vbslot, *slottab, *stptr;
extern struct _VBTAB       vt, *vtp, *vtabp;
extern struct _NTAB_STRUCT nountab, *ntab, *ntabp;
extern struct _TT_ENT      tt, *ttp, *ttabp;

#endif  // AMUL_H_AMULINC_H
