#include "context.h"
#include "amulcom.includes.h"

using namespace AMUL::Logging;

namespace Compiler
{
Context&
GetContext()
{
    static Context context;
    return context;
}

Context::~Context()
{
    /// TODO: Do things from main that are done based on m_completed?
}
}  // namespace Compiler