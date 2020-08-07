#pragma once
#ifndef AMUL_CLIENT_IO_H
#define AMUL_CLIENT_IO_H

#include <cstdio>  // For snprintf

#include "amul.typedefs.h"
#include "game.h"
#include "textline.h"

extern thread_local IoType t_iosup;  // Whether we're using console/network/logging output
extern thread_local bool t_addCR, t_needCR;

void Print(const char *s);
void Printc(char);

static inline void
Print(stringid_t msgId)
{
    Print(GetString(msgId));
}

static inline void
Print(string_view text) noexcept
{
    Print(text.data());
}

#define StdOut                                                                                     \
    TextLine { Print }

template<typename... Args>
void
Printf(string_view format, Args &&... args) noexcept
{
    char text[1024];
    snprintf(text, sizeof(text), format.data(), forward<Args>(args)...);  // see includes
    Print(text);
}

template<typename T>
void
LnPrint(T &&t)
{
    Printc('\n');
    Print(forward<T>(t));
}

void GetInput(char *into, size_t maxLength);
template<size_t Size>
void
SafeInput(char (&into)[Size])
{
    return GetInput(into, Size);
}

void Ansify(const char *ansiCode);

void PrintSlot(slotid_t slot, const char *text);
void PrintSlot(slotid_t slot, const char *fmt, int n);

char Prompt(stringid_t msg, const char *options);
void pressret();

#endif  // AMUL_CLIENT_IO_H
