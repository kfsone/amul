#include "h/amul.logging.h"

namespace AMUL::Logging
{
Logger &
GetLogger()
{
    static Logger logger;
    return logger;
}

Logger::~Logger() {}

}  // namespace AMUL::Logging
