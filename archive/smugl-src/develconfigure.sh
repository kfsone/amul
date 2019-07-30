#! /bin/sh
# $Id: develconfigure,v 1.3 1999/06/08 15:35:20 oliver Exp $
# Small script to facilitate development compilation;
# enables the whole books worth of GCC warnings.
# If you don't have GCC, you can't use this (atm)

# For linux:
spam_warnings="-Wold-style-cast -Weffc++"
warnings="\
    -Wcast-qual \
    -Wwrite-strings \
    -Wmissing-declarations \
    -Wconversion-null  \
    -Wctor-dtor-privacy \
    -Wdelete-non-virtual-dtor \
    -Wliteral-suffix \
    -Wmultiple-inheritance \
    -Wnamespaces \
    -Wnarrowing \
    -Wnoexcept \
    -Wnoexcept-type \
    -Wnon-virtual-dtor \
    -Wreorder \
    -Wregister \
    -Wstrict-null-sentinel \
    -Wno-non-template-friend \
    -Woverloaded-virtual \
    -Wno-pmf-conversions \
    -Wsign-promo \
    -Wvirtual-inheritance \
    -Wshadow \
    -Wsequence-point \
    -Wuseless-cast \
"
sanitizer="-fsanitize=address -fsanitize=undefined -fsanitize=vptr -fsanitize=enum -fsanitize=bool -fsanitize=nonnull-attribute -fsanitize=returns-nonnull-attribute -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=object-size -fsanitize=alignment -fsanitize=bounds-strict -fsanitize=signed-integer-overflow -fsanitize=return -fsanitize=null -fsanitize=vla-bound -fsanitize=unreachable -fsanitize=integer-divide-by-zero -fsanitize=shift-base -fsanitize=shift-exponent -fsanitize=shift"
compiler_flags="-DDEBUG -std=c++17 -O0 -g3 -Wall -Wextra $warnings $sanitizer"

CFLAGS="${compiler_flags}" CXXFLAGS="${compiler_flags}" ./configure

