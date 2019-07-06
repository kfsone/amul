#pragma once

extern const char* rflag[NRFLAGS];
extern const char* obflags1[NOFLAGS];
extern const char* obparms[NOPARMS];
extern const char* obflags2[NSFLAGS];
extern const char* conds[NCONDS];
extern const char tcop[NCONDS][3];
extern const char* acts[NACTS];
extern const char tacp[NACTS][3];
extern const char* syntax[NSYNTS];
extern const short int syntl[NSYNTS];

int isrflag(const char *s);
int isoflag1(const char *s);
int isoparm();
int isoflag2(const char *s);