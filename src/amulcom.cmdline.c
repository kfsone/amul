#include <stdio.h>
#include <sys/stat.h>
#if defined(_MSC_VER)
#    include <direct.h>
#else
#    include <unistd.h>
#endif

#include "amulcom.h"
#include "modules.h"
#include "system.h"

#include <h/amul.argp.h>
#include <h/amul.alog.h>

// For systems that support it: CTRL-C handler.
void
CXBRK()
{
    fprintf(stderr, "*** CTRL-C pressed: terminating\n");
    Terminate(EINTR);
}

error_t
misuse(const char **argv, const char *issue, const char *arg, error_t err)
{
    fprintf(stderr, "%s: %s: %s\n", argv[0], issue, arg);
    fprintf(stderr, "Try '%s -help' for usage information\n", argv[0]);
    fflush(stderr);
    return err;
}

error_t
usage(const char **argv)
{
    printf("Usage: %s [-h|-?] [-d] [-v|-q] [-r] [game directory]\n", argv[0]);
    printf("Compiler for AMUL multi-player games.\n");
    printf("\n");
    printf("  -help, -?                 Displays this help information\n");
    printf("  -d, -dmoves               Enables 'dmove' checking\n");
    printf("  -q, -quiet                Decreases output verbosity\n");
    printf("  -r, -room-reuse           Re-uses previous room data during compilation\n");
    printf("  -v, -verbose              Increases output verbosity\n");
    printf("  <game directory>          Optional path game files and data will be in\n");

    exit(0);
}

error_t
ParseCommandLine(const struct CommandLine *cmdline)
{
    const int    argc = cmdline->argc;
    const char **argv = cmdline->argv;
    struct stat  sb;

    enum LogLevel desiredLogLevel = AL_INFO;

    for (int n = 1; n < argc; n++) {
        const char *arg = argv[n];
        if (arg[0] == '-') {
            if (strncmp("-h", arg, 2) == 0 || strncmp("--h", arg, 2) == 0 || strcmp("-?", arg) == 0)
                return usage(argv);

            if (strncmp("-d", arg, 2) == 0) {
                checkDmoves = true;
                continue;
            }
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
            if (strcmp("-r", arg) == 0) {
                reuseRoomData = true;
                continue;
            }
            return misuse(argv, "Unrecognized argument", arg, EINVAL);
        }
        if (gameDir[0] != 0)
            return misuse(argv, "Unexpected argument", arg, EINVAL);
        if (PathCopy(gameDir, sizeof(gameDir), NULL, arg) != 0)
            return misuse(argv, "Invalid game directory", arg, EINVAL);
        int err = stat(gameDir, &sb);
        if (err != 0)
            return misuse(argv, "Missing game directory/no access", gameDir, err);
        if (S_ISREG(sb.st_mode))
            return misuse(argv, "Not a directory", gameDir, EINVAL);
    }

    char pwd[MAX_PATH_LENGTH];
    if (getcwd(pwd, sizeof(pwd)) == NULL) {
        alog(AL_FATAL, "Cannot get CWD");
    }
    if (strncmp(gameDir, "./", 2) == 0) {
        path_concater(pwd, gameDir + 2);
        gameDir[0] = 0;
    }
    if (gameDir[0] == 0 || strcmp(gameDir, ".") == 0) {
        path_copier(gameDir, pwd);
    }

    alogLevel(desiredLogLevel);

    return 0;
}

error_t
initCommandLine(struct Module *module)
{
    return ParseCommandLine((struct CommandLine *)(module->context));
}

error_t
InitCommandLine(const struct CommandLine *cmdline)
{
    return NewModule(false, MOD_CMDLINE, initCommandLine, NULL, NULL, (void *)cmdline, NULL);
}
