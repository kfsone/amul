#include <cstdio>
#include <cstring>

#include "amul.argp.h"
#include "amul.file.h"
#include "filesystem.h"
#include "logging.h"
#include "modules.h"
#include "system.h"

error_t
usage(const char **argv)
{
    printf("Usage: %s [-h|-?] [-v|-q] [game directory]\n", argv[0]);
    printf("Compiler for AMUL multi-player games.\n");
    printf("\n");
    printf("  -help, -?                 Displays this help information\n");
    printf("  -q, -quiet                Decreases output verbosity\n");
    printf("  -v, -verbose              Increases output verbosity\n");
    printf("  <game directory>          Optional path game files and data will be in\n");

    exit(0);
}

error_t
ParseCommandLine(const CommandLine *cmdline)
{
    const int argc = cmdline->argc;
    const char **argv = cmdline->argv;
    struct stat sb;

    for (int n = 1; n < argc; n++) {
        const char *arg = argv[n];
        if (arg[0] == '-') {
            if (strncmp("-h", arg, 2) == 0 || strncmp("--h", arg, 2) == 0 || strcmp("-?", arg) == 0)
                return usage(argv);
            if (strncmp("-q", arg, 2) == 0) {
                if (g_desiredLogLevel < MAX_LOG_LEVEL - 1)
                    ++g_desiredLogLevel;
                continue;
            }
            if (strncmp("-v", arg, 2) == 0) {
                if (g_desiredLogLevel > 0)
                    --g_desiredLogLevel;
                continue;
            }
            return CmdlineMisuse(argv, "Unrecognized argument", arg, EINVAL);
        }
        if (!gameDir.empty())
            return CmdlineMisuse(argv, "Unexpected argument", arg, EINVAL);
        PathAdd(gameDir, arg);
        if (auto err = stat(gameDir.c_str(), &sb); err != 0)
            return CmdlineMisuse(argv, "Missing game directory/no access", gameDir.c_str(), err);
        if (S_ISREG(sb.st_mode))
            return CmdlineMisuse(argv, "Not a directory", gameDir.c_str(), EINVAL);
    }

    char pwd[MAX_PATH_LENGTH];
    if (!getcwd(pwd, sizeof(pwd)))
        LogFatal("Cannot get CWD");

    if (gameDir.substr(0, 2) == "./") {
        PathJoin(gameDir, pwd, gameDir.substr(2));
    }
    if (gameDir.empty()) {
#if defined(AMUL_DEFAULT_GAME)
        gameDir = AMUL_DEFAULT_GAME;
#else
        gameDir = ".";
#endif
    }
    if (gameDir == ".") {
        gameDir = pwd;
    }

    return 0;
}