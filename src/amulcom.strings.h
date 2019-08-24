#ifndef AMUL_SRC_AMULCOM_STRINGS_H
#define AMUL_SRC_AMULCOM_STRINGS_H 1

#include <h/amul.strs.h>
#include <h/amul.type.h>

#include <cstdio>
#include <string_view>

error_t InitStrings();

std::string &StringLower(std::string &str);

size_t GetStringCount();
size_t GetStringBytes();

error_t AddTextString(std::string_view text, bool isLine, stringid_t *idp);

error_t RegisterTextString(
        std::string_view label, std::string_view text, bool isLine, stringid_t *idp);

error_t TextStringFromFile(const char *label, FILE *fp, stringid_t *idp, bool toEol=false);

error_t LookupTextString(std::string_view label, stringid_t *idp=nullptr);

#endif
