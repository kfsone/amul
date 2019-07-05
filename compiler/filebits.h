#pragma once

#include <cstdint>
#include <cstdio>

void	close_ofps();
char	nextc(int f);
void	quit();
FILE *  fopenw(char *s);
FILE *  fopena(char *s);
FILE *  fopenr(char *s);
void	Err(char *s, char *t);
int32_t rfopen(char *s);
void	ttroomupdate();
void	opentxt(char *s);
void	skipblock();
void	tidy(char *s);
int		is_verb(char *s);
void	blkget(int32_t *s, char **p, int32_t off);
int32_t filesize();
