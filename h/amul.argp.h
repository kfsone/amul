#ifndef AMUL_H_AMUL_ARGP_H
#define AMUL_H_AMUL_ARGP_H

#include <h/amul.type.h>

// Argument parsing.
struct CommandLine {
    int          argc;
    const char **argv;
    const char **envp;
};

extern error_t InitCommandLine(const struct CommandLine *cmdline);

#endif