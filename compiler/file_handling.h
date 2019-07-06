#pragma once

#include <cstdint>
#include <cstdio>

void	close_ofps();
char	nextc(int f);
[[noreturn]] void quit();
FILE	*fopenw(const char *s);
FILE	*fopena(const char *s);
FILE	*fopenr(const char *s);
FILE	*rfopen(const char *s);
void	ttroomupdate();
void	 opentxt(const char *s);
void	skipblock();
void	tidy(char *s);
int		is_verb(const char *s);
void	blkget(int32_t *s, char **p, int32_t off);
int32_t filesize();
