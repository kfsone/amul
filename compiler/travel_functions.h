#pragma once

#include <cstdint>
#include <cstdio>

const char *precon(const char *s);
const char *preact(const char *s);
int32_t     chknum(const char *p);
const char *chkp(const char *p, char t, int c, int z, FILE *fp);
int         isgender(char c);
int         antype(const char *s);
int         isnounh(const char *s);
int         rdmode(char c);
int         isspell(const char *s);
int         isstat(const char *s);
int         bvmode(char c);
const char *chkaparms(const char *p, int c, FILE *fp);
const char *chkcparms(const char *p, int c, FILE *fp);
int         onoff(const char *p);
int         actualval(const char *s, int n);
