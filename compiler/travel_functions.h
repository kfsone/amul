#pragma once

#include <cstdint>
#include <cstdio>

const char *precon(const char *s) noexcept;
const char *preact(const char *s) noexcept;
int32_t     chknum(const char *p) noexcept;
const char *chkp(const char *p, char t, int c, int z, FILE *fp) noexcept;
int         isgender(char c) noexcept;
int         antype(const char *s) noexcept;
int         isnounh(const char *s);
int         rdmode(char c);
int         isspell(const char *s);
int         isstat(const char *s);
int         bvmode(char c);
const char *chkaparms(const char *p, int c, FILE *fp);
const char *chkcparms(const char *p, int c, FILE *fp);
int         onoff(const char *p);
int         actualval(const char *s, int n);
