#include "amulcom.h"
#include "modules.h"

#include <h/amul.argp.h>
#include <h/amul.alog.h>
#include <h/amul.test.h>

int
main(int argc, const char **argv)
{
    struct CommandLine cmdline = (struct CommandLine){argc, argv, NULL};

    InitModules();

	ERROR_CHECK(InitLogging());
	ERROR_CHECK(InitCommandLine(&cmdline));

    error_t retval = amulcom_main();

    CloseModules(retval);

	return retval;
}
