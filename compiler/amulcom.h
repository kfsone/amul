#pragma once

// amulcom misc prototypes

void checkdmoves();
int  isroom(const char *name);
int  isnoun(const char *name);
int  iscont(const char *name);
int  isloc(const char *name);
int  isprep(const char *s);
int  ttumsgchk(const char *s);
int  chkumsg(const char *s);
void set_adj();
void object(const char *s);
void set_start();
void set_holds();
void set_put();
void set_mob();

constexpr bool
isCommentChar(char c) noexcept
{
    return c == ';';
}

constexpr bool
isEol(char c) noexcept
{
    return c == '\n';
}

// Lines end when we reach the nul terminator, an eol character or a comment character.
constexpr bool
isLineEnding(char c) noexcept
{
    return (c == 0 || isEol(c) || isCommentChar(c));
}

constexpr bool
isLineBreak(char c) noexcept
{
    return (c == 0 || isCommentChar(c));
}