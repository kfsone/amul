#include "amulcom.h"

#include "modules.h"

int
main(int argc, const char **argv)
{
    struct CommandLine cmdline = (struct CommandLine){argc, argv, NULL};
    InitModules(&cmdline);
	error_t retval = amulcom_main();
    CloseModules(retval);
	return retval;
}
