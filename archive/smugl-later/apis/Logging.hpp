#pragma once

// Defines a logging interface, to be implemented in a platform-specific fashion, but provide a
// platform-agnostic interface.

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <string>

// Helper macros; example usage:
//	log->Write(_FLI, "This is informatio about %s", someString);
#define _FLD __FILE__, __LINE__, APIs::Logging::Debug  // Developer-level runtime information.
#define _FLM __FILE__, __LINE__, APIs::Logging::Minor  // Minor warning relevant only to developers.
#define _FLI __FILE__, __LINE__, APIs::Logging::Info   // Normal runtime events/activity.
#define _FLW                                                                                       \
    __FILE__, __LINE__, APIs::Logging::Warning  // Warning that is not guaranteed to be significant.
#define _FLE                                                                                       \
    __FILE__, __LINE__, APIs::Logging::Error  // Bad behavior that the code is dealing with.
#define _FLT                                                                                       \
    __FILE__, __LINE__, APIs::Logging::Terminal  // Bad behavior that caused the code to exit.
#define _FLF                                                                                       \
    __FILE__, __LINE__, APIs::Logging::Fatal  // Internal bad behavior that is causing a core dump.

namespace APIs
{
namespace Logging
{
//! Logging levels, where 'Terminal' results in an app shutdown.
enum Level { Disabled, Debug, Info, Minor, Warning, Error, Terminal, Fatal, MaxLevel };

class Log
{
  public:
  public:
    //! Main constructor.
    //! @param[in]	name_			How the log entries will be identified either as a filename or a
    //! logging level, or possible both.
    //! @param[in]	minLevel_		Minimum log-level to record to this mechanism.
    Log(const wchar_t* const name_, const APIs::Logging::Level minLevel_);

  public:
    //! Destructor.
    ~Log();

  public:
    //! Change logging level.
    //! @param[in]	newLevel_		Minimum log-level that will be recorded subsequently.
    void ChangeLevel(const APIs::Logging::Level newLevel_);

  public:
    //! Check if a message of level N will be logged.
    //! Allows you to optimize out expensive calls involved in preparing log
    //! messages when the level would be below current reporting.
    //! @result true if the specified logging level will write to the log, false otherwise.
    bool WillLog(const APIs::Logging::Level level_) const
    {
        return (m_currentLevel > APIs::Logging::Disabled) & (m_currentLevel >= level_);
    }

  public:
    //! Write something to the log file.
    //! @param[in]	srcFileName_	Origin of the error, usually __FILE__
    //! @param[in]	srcLineNo_		Origin line within srcFileName_ of the error, i.e. __LINE__
    //! @param[in]	level_			Logging::Level at which to record the message
    //! @param[in]	format_			"sprintf" style format mask
    void Write(const char* const srcFilename_,
               const unsigned int srcLineNo_,
               const APIs::Logging::Level level_,
               const char* const format_,
               ...)
    {
        if (WillLog(level_) == false)
            return;
        va_list args;
        va_start(args, format_);
        vWrite(srcFilename_, srcLineNo_, level_, format_, args);
        va_end(args);
    }

    //! Write something to the log file, followed by the description of the last system error
    //! (perror).
    //! @param[in]	srcFileName_	Origin of the error, usually __FILE__
    //! @param[in]	srcLineNo_		Origin line within srcFileName_ of the error, i.e. __LINE__
    //! @param[in]	level_			Logging::Level at which to record the message
    //! @param[in]	prefix_			Prefix message.
    void Perror(const char* const srcFilename_,
                const unsigned int srcLineNo_,
                const APIs::Logging::Level level_,
                const char* const prefix_,
                ...)
    {
        Write(srcFilename_, srcLineNo_, level_, "%s: %s", prefix_, strerror(errno));
    }

  public:
    //! Overload() to perform a write.
    //! @param[in]	srcFileName_	Origin of the error, usually __FILE__
    //! @param[in]	srcLineNo_		Origin line within srcFileName_ of the error, i.e. __LINE__
    //! @param[in]	level_			Logging::Level at which to record the message
    //! @param[in]	format_			"sprintf" style format mask
    void operator()(const char* const srcFilename_,
                    const unsigned int srcLineNo_,
                    const APIs::Logging::Level level_,
                    const char* const format_,
                    ...)
    {
        if (WillLog(level_) == false)
            return;
        va_list args;
        va_start(args, format_);
        vWrite(srcFilename_, srcLineNo_, level_, format_, args);
        va_end(args);
    }

  private:
    Log() {}                     // Default constructor prohibited.
    Log(const Log&) {}           // Copy constructor prohibited.
    Log& operator=(const Log&);  // Copy operator prohibited.

    void vWrite(const char* const srcFilename_,
                const unsigned int srcLineNo_,
                const APIs::Logging::Level level_,
                const char* const format_,
                va_list& args);

  private:
    std::wstring m_name;            // File/level name of the log.
    Logging::Level m_currentLevel;  // Our current logging level.
};

}  // namespace Logging

}  // namespace APIs
