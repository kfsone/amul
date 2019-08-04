/*
 * Platform agnostic logging facility.
 */

#include "syslog.hpp"

namespace APIs::Logging
{

//////////////////////////////////////////////////////////////////////
// You have to declare it in your own module so you can supply
// the correct module name.
extern Log sysLog;

//////////////////////////////////////////////////////////////////////
// Constructor.

Log::Log(const wchar_t *const name_, const APIs::Logging::Level minLevel_)
    : m_name(name_), m_currentLevel(minLevel_)
{
}

//////////////////////////////////////////////////////////////////////
// Destructor.

Log::~Log() = default;

}  // namespace APIs::Logging
