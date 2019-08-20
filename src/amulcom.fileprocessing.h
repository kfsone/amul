#ifndef AMULCOM_FILEPROCESSING_H
#define AMULCOM_FILEPROCESSING_H

#include <cstdio>

extern thread_local FILE *ifp;

extern char Word[64];  /// TODO: Pass

bool nextc(bool required);
char *getTidiedLineToScratch(FILE *fp);
void tidy(char *ptr);
void skipParagraph();
void CheckErrorCount();

#endif  // AMULCOM_FILEPROCESSING_H
