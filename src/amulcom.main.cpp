#include "h/amul.argp.h"
#include "h/amul.test.h"
#include "h/logging.h"
#include "h/modules.h"

extern int amulcom_main();

int
main(int argc, const char **argv)
{
    CommandLine cmdline = { argc, argv, nullptr };

    InitModules();

    ERROR_CHECK(InitLogging());
    ERROR_CHECK(InitCommandLine(&cmdline));

    const error_t retval = amulcom_main();

    CloseModules(retval);

    return retval;
}
