#include "h/amul.logging.h"

extern void quit();

namespace AMUL::Logging {

Logger &
GetLogger()
{
	static Logger logger;
	return logger;
}

}  // namespace AMUL::Logging
