#include "amulcom.includes.h"
#include "context.h"

using namespace AMUL::Logging;

namespace Compiler
{

Compiler::Context&
GetContext()
{
	static Context context;
	return context;
}

Context::~Context()
{
	/// TODO: Do things from main that are done based on m_completed?
}

}