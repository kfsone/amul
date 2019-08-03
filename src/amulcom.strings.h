#ifndef AMUL_SRC_AMULCOM_STRINGS_H
#define AMUL_SRC_AMULCOM_STRINGS_H 1

#include <h/amul.strs.h>
#include <h/amul.type.h>

#include <stdbool.h>
#include <stdio.h>

error_t InitStrings();

size_t GetStringCount();
size_t GetStringBytes();

error_t AddTextString(const char *start, const char *end, bool isLine, stringid_t *idp);

error_t RegisterTextString(
        const char *label, const char *start, const char *end, bool isLine, stringid_t *idp);

error_t TextStringFromFile(const char *label, FILE *fp, stringid_t *idp, bool toEol=false);

error_t LookupTextString(const char *label, stringid_t *idp);

#endif
