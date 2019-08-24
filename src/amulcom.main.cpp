#include "amulcom.h"
#include "logging.h"
#include "modules.h"

#include <h/amul.argp.h>
#include <h/amul.test.h>

int
main(int argc, const char **argv)
{
    CommandLine cmdline = {argc, argv, nullptr};

    InitModules();

    ERROR_CHECK(InitLogging());
    ERROR_CHECK(InitCommandLine(&cmdline));

    const error_t retval = amulcom_main();

    CloseModules(retval);

    return retval;
}
