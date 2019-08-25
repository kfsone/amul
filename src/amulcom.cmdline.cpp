#include <cstdio>
#include <cstring>

#include "h/amul.argp.h"
#include "h/amul.file.h"
#include "h/filesystem.h"
#include "h/logging.h"
#include "h/modules.h"
#include "h/system.h"

// For systems that support it: CTRL-C handler.
void
CXBRK()
{
    LogError("*** CTRL-C pressed: terminating");
    Terminate(EINTR);
}

error_t
misuse(const char **argv, const char *issue, const char *arg, error_t err)
{
    LogError(argv[0], ": ", issue, ": ", arg, "\n",
            "Try '", argv[0], " -help' for usage information.\n");
    return err;
}

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
    const int    argc = cmdline->argc;
    const char **argv = cmdline->argv;
    struct stat  sb;

    int desiredLogLevel = LWARN;

    for (int n = 1; n < argc; n++) {
        const char *arg = argv[n];
        if (arg[0] == '-') {
            if (strncmp("-h", arg, 2) == 0 || strncmp("--h", arg, 2) == 0 || strcmp("-?", arg) == 0)
                return usage(argv);
            if (strncmp("-q", arg, 2) == 0) {
                if (desiredLogLevel < MAX_LOG_LEVEL - 1)
                    ++desiredLogLevel;
                continue;
            }
            if (strncmp("-v", arg, 2) == 0) {
                if (desiredLogLevel > 0)
                    --desiredLogLevel;
                continue;
            }
            return misuse(argv, "Unrecognized argument", arg, EINVAL);
        }
        if (gameDir[0] != 0)
            return misuse(argv, "Unexpected argument", arg, EINVAL);
        if (PathCopy(gameDir, sizeof(gameDir), nullptr, arg) != 0)
            return misuse(argv, "Invalid game directory", arg, EINVAL);
        int err = stat(gameDir, &sb);
        if (err != 0)
            return misuse(argv, "Missing game directory/no access", gameDir, err);
        if (S_ISREG(sb.st_mode))
            return misuse(argv, "Not a directory", gameDir, EINVAL);
    }

    char pwd[MAX_PATH_LENGTH];
    if (!getcwd(pwd, sizeof(pwd)))
        LogFatal("Cannot get CWD");

    if (strncmp(gameDir, "./", 2) == 0) {
        path_concater(pwd, gameDir + 2);
        gameDir[0] = 0;
    }
    if (gameDir[0] == 0 || strcmp(gameDir, ".") == 0) {
        path_copier(gameDir, pwd);
    }

    SetLogLevel((LogLevel)desiredLogLevel);

    return 0;
}

error_t
initCommandLine(Module *module)
{
    return ParseCommandLine((CommandLine *)(module->context));
}

error_t
InitCommandLine(const CommandLine *cmdline)
{
    return NewModule(MOD_CMDLINE, initCommandLine, nullptr, nullptr, (void *) cmdline, nullptr);
}
