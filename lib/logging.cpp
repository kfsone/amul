#include "h/amul.logging.h"

[[noreturn]] void quit();

namespace AMUL::Logging
{
Logger&
GetLogger()
{
    static Logger logger;
    return logger;
}

Logger::~Logger() {}

}  // namespace AMUL::Logging
