#pragma once
#ifndef AMUL_AMUL_STRINGMANIP_H
#define AMUL_AMUL_STRINGMANIP_H

#include <string>
#include <string_view>

#include "game.h"
#include "typedefs.h"

std::string MakeTitle(slotid_t slotId);
void PutARankInto(char *into, slotid_t slot) noexcept;
void PutRankInto(char *into) noexcept;
bool ProcessEscape(const char *escape, char *s);
char *ProcessString(const char *s);
const char *GetTimeStr(time_t timenow = 0) noexcept;
void FormatTimeInterval(char *into, uint32_t seconds) noexcept;
void SetMxxMxy(BroadcastType style, slotid_t slotid) noexcept;
string_view Trim(string_view *view) noexcept;
std::string Meld(string_view prefix, string_view content, string_view suffix);
constexpr auto JoinStrings =
        [](auto cur, auto end, string_view join = ", ", bool oxford = false) {
            if (cur + 1 == end)
                return std::string{ *cur };
            std::string result{ *(cur++) };
            auto penultimate = end - 1;
            while (cur != penultimate) {
                result += join;
                result += *(cur++);
            }
            result += join;
            if (oxford)
                result += "and ";
            result += *cur;
            return result;
        };

#endif  // AMUL_AMUL_STRINGMANIP_H

