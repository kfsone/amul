#ifndef AMUL_SRC_AMULCOM_FILEPROCESSING_H
#define AMUL_SRC_AMULCOM_FILEPROCESSING_H

#include <cstdio>
extern FILE *ifp;

extern char Word[64];    ///TODO: Pass

void fopenr(const char *filename);
bool nextc(bool required);
char *getTidyBlock(FILE *fp);
void tidy(char *ptr);
void skipblock();
void checkErrorCount();

#endif  // AMUL_SRC_AMULCOM_FILEPROCESSING_H
