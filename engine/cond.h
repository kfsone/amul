#pragma once

char CHAEtype(int objectNo);
int  cangive(int objectNo, int playerNo);
int  cangive(int objectNo, int playerNo);
int  cansee(int viewer, int subject);
int  canseeobj(int objectNo, int who);
int  carrying(int objectNo);
int  infl(int playerNo, int spell);
int  isanoun(const char *s);
int  isaverb(char **s);
int  isin(int objectNo, int roomNo);
int  isnoun(const char *s, int adjective, const char *pattern);
int  isprep(const char *s);
int  isroom(const char *s);
int  issyn(const char *s);
int  isverb(const char *s);
int  lit(int roomNo);
int  loc(int objectNo);
int  magic(int rank, int points, int chance);
int  nearto(int objectNo);
int  scan(int start, char scanType, int test, const char *s, int adj);
int  stat(int playerNo, int statNo, int value);
int  visible();
