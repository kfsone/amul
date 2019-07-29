#! /bin/sh
# $Id: develconfigure,v 1.3 1999/06/08 15:35:20 oliver Exp $
# Small script to facilitate development compilation;
# enables the whole books worth of GCC warnings.
# If you don't have GCC, you can't use this (atm)

# For linux:
compiler_flags="-DDEBUG -std=c++14 -O0 -g3 -Wall -Wcast-qual -Wwrite-strings -Wmissing-declarations"
CFLAGS="${compiler_flags}" CXXFLAGS="${compiler_flags}" ./configure
# If you REALLY want to be pedantic
#CFLAGS="-DDEBUG -g -Wall -Wshadow -Wstrict-prototypes -Wcast-qual -Wwrite-strings -Wconversion -Wmissing-prototypes -Wmissing-declarations" ./configure
