// OS Utilities

#include "h/amul.incs.h"
#include <cstdarg>
#include <cstdlib>

namespace OS {

void
SetProcessName(const char *title)
{
	// Original Amiga implementation
#if defined(AMIGA)
	Task *myTask = FindTask(nullptr);
	if (myTask) {
		myask->tc_Node.ln_Name = title;
	}
#else
	(void)title;
	/// TODO: Implement
#endif
}

int
Run(const char *cmd, ...)
{
	char cmdBuffer[256];

	va_list argp;
	va_start(argp, cmd);

	vsnprintf(cmdBuffer, sizeof(cmdBuffer), cmd, argp);

	va_end(argp);

#if defined(AMIGA)
	vsnprintf(block, sizeof(block), "run >NIL: %s", cmdBuffer);
	return Execute(block, 0, 0);
#else
	return system(cmdBuffer);
#endif
}

}  // namespace OS