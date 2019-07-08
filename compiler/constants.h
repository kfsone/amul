#pragma once

#include <string>

extern const char *    rflag[NRFLAGS];
extern const char *    obflags1[NOFLAGS];
extern const char *    obparms[NOPARMS];
extern const char *    obflags2[NSFLAGS];
extern const char *    syntax[NSYNTS];
extern const short int syntl[NSYNTS];

int isrflag(const char *s) noexcept;
int isoflag1(const char *s) noexcept;
int isoparm() noexcept;
int isoflag2(const char *s) noexcept;

condid_t getCondition(std::string token) noexcept;
actionid_t getAction(std::string token) noexcept;
