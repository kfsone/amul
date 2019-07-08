#pragma once

#include "h/amul.logging.h"

namespace Compiler
{
// Describes the context of the current compilation.
struct Context {
    ~Context();

    // true when the compiler has finished what it is doing, so it is
    // safe to discard temporaries, etc.
    // Used in quit()
    /// TODO: Replace with RAII containers in compiler main
    bool m_completed{false};

    // Don't check dmoves
    bool m_skipDmoveCheck{false};

    // Skip reading rooms and use previous data
    bool m_skipRoomRead{false};

    //////

    // Get error count
    size_t errorCount() const { return AMUL::Logging::GetLogger().m_numErrors; }

    constexpr size_t errorLimit() const { return 30; }

    void addError() { AMUL::Logging::GetLogger().m_numErrors++; }

    // Check the error limit and terminate if it's been exceeded
    void checkErrorCount() const
    {
        if (errorCount() > errorLimit()) {
            AMUL::Logging::GetLogger().fatalf(
                    "Exceeded maximum number of errors");
        }
    }

    // Terminate if there are greater-than-zero errors
    void terminateOnErrors() const
    {
        if (errorCount()) {
            AMUL::Logging::GetLogger().fatalf(
                    "Aborting due to %u errors.", errorCount());
        }
    }
};

Context &GetContext();

}  // namespace Compiler