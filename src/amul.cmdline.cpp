#include "amulconfig.h"

#include <cctype>
#include <cstdio>
#include <cstring>

#include "amul.argp.h"
#include "amul.defs.h"
#include "typedefs.h"
#include "amul.vars.h"
#include "filesystem.h"
#include "logging.h"
#include "modules.h"

#if defined(AMUL_DEFAULT_GAME)
constexpr const char DefaultGamePath[] = AMUL_DEFAULT_GAME;
#else
constexpr const char DefaultGamePath[] = ".";
#endif

extern void asend(int type, int data);

static void
shutreq(bool kill, unsigned int seconds)
{
#ifdef MESSAGE_CODE
    asend(kill ? MKILL : MRESET, seconds);
#else
	(void)kill;
	(void)seconds;
#endif
}

static void
sendext(unsigned int seconds)
{
#ifdef MESSAGE_CODE
    asend(MEXTEND, seconds);
#else
	(void)seconds;
#endif
}

[[noreturn]] static void
executeCommand(int argc, const char *argv[])
{
    if (!t_managerPort) {
        LogFatal("AMAN is not running");
    }
    unsigned int seconds{ 0 };
    if (argc == 3)
        sscanf(argv[2], "%u", &seconds);
    switch (toupper(*(argv[1] + 1))) {
        case 'K':
            shutreq(true, seconds);
            break;
        case 'R':
            shutreq(false, seconds);
            break;
        case 'X':
            if (argc != 3)
                LogFatal("Missing minute value after -x option");
            sendext(seconds);
            break;
    }
    exit(0);
}

[[noreturn]] static void
usage(const char *argv[], error_t err)
{
    printf("Usage: %s [-h|-?|--help] [-v|--verbose] [-q|--quiet] [game directory]\n", argv[0]);
    printf("Server for AMUL multi-player games.\n");
    printf("\n");
    printf("  --help               Displays this help information\n");
    printf("  --quiet              Decreases output verbosity\n");
    printf("  --verbose            Increases output verbosity\n");
    printf("  <game directory>     Option path containing game files\n");

    exit(err);
}

error_t
ParseCommandLine(const CommandLine *cmdline)
{
    const int argc = cmdline->argc;
    const char **argv = cmdline->argv;

    if (argc > 1 && argv[1][0] == '-' && strchr("krc", argv[1][1])) {
        executeCommand(argc, argv);
        return EINVAL;
    }

    for (int argn = 1; argn < argc; ++argn) {
        string_view arg = argv[argn];
        if (arg[0] == '-') {
            if (arg == "-v" || arg == "--verbose") {
                if (g_desiredLogLevel > 0)
                    --g_desiredLogLevel;
                continue;
            }
            if (arg == "-q" || arg == "--quiet") {
                g_quiet = true;
                if (g_desiredLogLevel < MAX_LOG_LEVEL - 1)
                    ++g_desiredLogLevel;
                continue;
            }
            if (arg == "-h" || arg == "-?" || arg == "--help") {
                usage(argv, 0);
            }
            printf("ERROR: Invalid command line argument: %s", arg.data());
            usage(argv, EINVAL);
        }

        if (!gameDir.empty()) {
            LogFatal("Invalid argument/multiple paths specified: %s", arg.data());
        }

        PathAdd(gameDir, arg.data());
    }

	if (gameDir.empty())
        gameDir = DefaultGamePath;

    return 0;
}
