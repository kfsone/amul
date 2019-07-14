#include "amulcom.h"
#include "modules.h"

#include <h/amul.argp.h>
#include <h/amul.alog.h>

int
main(int argc, const char **argv)
{
    struct CommandLine cmdline = (struct CommandLine){argc, argv, NULL};

    InitModules();

    InitLogging();

    error_t err = InitCommandLine(&cmdline);
    if (err != 0)
        return err;

    error_t retval = amulcom_main();

    CloseModules(retval);

	return retval;
}
