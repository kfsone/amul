// OS Utilities

#include "h/amul.incs.h"
#include "h/amul.logging.h"

#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <fcntl.h>
#include <string>

#if !defined(_MSC_VER)
#    include <unistd.h>
#endif

using namespace AMUL::Logging;

namespace OS
{
void
CreateFile(const char *path, const char *filename)
{
    std::string filepath = path;
    filepath += "/";
    filepath += filename;

    int fd = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0660);
    if (fd == -1) {
        GetLogger().fatalf("Failed to create file: %s: %s", filepath.c_str(), strerror(errno));
    }
    close(fd);
}

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
