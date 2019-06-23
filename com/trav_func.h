#pragma once

#include <cstdint>
#include <cstdio>

const char *precon(const char *s);
const char *preact(const char *s);
int32_t		chknum(const char *p);
const char *optis(const char *p);
char *		chkp(const char *p, char t, int c, int z, FILE *fp);
int			isgender(char c);
int			antype(const char *s);
int			isnounh(const char *s);
int			rdmode(char c);
int			isspell(const char *s);
int			isstat(const char *s);
int			bvmode(char c);
char *		chkaparams(char *p, int c, FILE *fp);
char *		chkcparams(char *p, int c, FILE *fp);
int			onoff(char *p);
