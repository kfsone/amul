#pragma once
#ifndef AMULCOM_STRINGS_H
#define AMULCOM_STRINGS_H 1

#include "amul.strs.h"
#include "typedefs.h"

error_t InitStrings() noexcept;

size_t GetStringCount() noexcept;
size_t GetStringBytes() noexcept;

error_t AddTextString(string_view text, bool isLine, stringid_t *idp);

error_t
RegisterTextString(string_view label, string_view text, bool isLine, stringid_t *idp);

error_t TextStringFromFile(const char *label, FILE *fp, stringid_t *idp, bool toEol = false);

error_t LookupTextString(string_view label, stringid_t *idp = nullptr);

#endif