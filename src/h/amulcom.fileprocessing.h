#ifndef AMULCOM_FILEPROCESSING_H
#define AMULCOM_FILEPROCESSING_H

#include <cstdio>

extern FILE *ifp;

extern char Word[64];    ///TODO: Pass

bool nextc(bool required);
char *getTidyBlock(FILE *fp);
void tidy(char *ptr);
void skipblock();
void checkErrorCount();

#endif  // AMULCOM_FILEPROCESSING_H
