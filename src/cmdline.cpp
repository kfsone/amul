#include "amul.argp.h"
#include "filesystem.h"
#include "logging.h"
#include "modules.h"

int g_desiredLogLevel = LWARN;

// For systems that support it: CTRL-C handler.
[[noreturn]]
void
CXBRK()
{
    LogError("*** CTRL-C pressed: terminating");
    Terminate(EINTR);
}

error_t
CmdlineMisuse(const char **argv, const char *issue, const char *arg, error_t err)
{
    LogError(argv[0],
             ": ",
             issue,
             ": ",
             arg,
             "\n",
             "Try '",
             argv[0],
             " -help' for usage information.\n");
    return err;
}

error_t
initCommandLine(Module *module)
{
    const CommandLine *cmdline = reinterpret_cast<CommandLine *>(module->context);
    if (error_t err = ParseCommandLine(cmdline); err != 0)
        return err;

    SetLogLevel(static_cast<LogLevel>(g_desiredLogLevel));

    LogInfo("Game Path: ", gameDir);

    return 0;
}

error_t
InitCommandLine(const CommandLine *cmdline)
{
	void *cmdlinePtr = reinterpret_cast<void*>(const_cast<CommandLine*>(cmdline));
    return NewModule(MOD_CMDLINE, initCommandLine, nullptr, nullptr, cmdlinePtr, nullptr);
}
