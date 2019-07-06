#pragma once

int               CXBRK();
void              memfail(const char *s);
void              givebackmemory();
void              fopenr(const char *s);
void              fopena(const char *s);
void              close_ofps();
void              kquit(const char *s);
[[noreturn]] void quit();
void              SendIt(int t, int d, char *p);
void              pressret();
void              sys(int sysMsgNo);
void              crsys(int sysMsgNo);
void              timeto(char *s, int32_t seconds);
