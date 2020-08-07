#ifndef AMUL_ARGP_H
#define AMUL_ARGP_H

#include "amul.typedefs.h"

// Argument parsing.
struct CommandLine {
    int argc;
    const char **argv;
    const char **envp;
};

extern int g_desiredLogLevel;

extern error_t InitCommandLine(const struct CommandLine *cmdline);

extern error_t CmdlineMisuse(const char **argv, const char *issue, const char *arg, error_t err);

extern error_t ParseCommandLine(const CommandLine *cmdline);

#endif
