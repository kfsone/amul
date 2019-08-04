#ifndef AMUL_SRC_AMULCOM_STRINGS_H
#define AMUL_SRC_AMULCOM_STRINGS_H 1

#include <h/amul.strs.h>
#include <h/amul.type.h>

#include <cstdbool>
#include <cstdio>

error_t InitStrings();

size_t GetStringCount();
size_t GetStringBytes();

error_t AddTextString(const char *start, const char *end, bool isLine, stringid_t *idp);

error_t RegisterTextString(
        const char *label, const char *start, const char *end, bool isLine, stringid_t *idp);
static inline
error_t RegisterTextString(
        const char *label, const char *start, size_t size, bool isLine, stringid_t *idp)
{
    return RegisterTextString(label, start, start + size, isLine, idp);
}

error_t TextStringFromFile(const char *label, FILE *fp, stringid_t *idp, bool toEol=false);

error_t LookupTextString(const char *label, stringid_t *idp=nullptr);

#endif
