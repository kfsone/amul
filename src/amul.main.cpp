#include "amul.argp.h"
#include "amul.test.h"
#include "logging.h"
#include "modules.h"

extern void amul_main();

extern error_t InitServer();
extern error_t InitClient();

extern void ConsoleMain();

int
main(int argc, const char **argv)
{
    CommandLine cmdline = { argc, argv, nullptr };

    InitModules();

    ERROR_CHECK(InitLogging());
    ERROR_CHECK(InitCommandLine(&cmdline));

    ERROR_CHECK(InitServer());
    ERROR_CHECK(InitClient());

    LogInfo("Starting Modules");
    ERROR_CHECK(StartModules());

    ConsoleMain();

    CloseModules(0);

    return 0;
}
