#include "amulcom.fileparser.h"
#include "amulcom.fileprocessing.h"
#include "stringmanip.h"

void
FileParser::Parse()
{
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting: Unable to open file: ", src.filepath, ": ", err);
    }

    while (!src.Eof()) {
        // Terminate if we've reached the error limit.
        CheckErrorCount();

		startBlock();

        // Check for the expected prefix and then tokenize the remaining line.
        if (!src.GetIDLine(idPrefix())) {
            continue;
        }

        // Convert the ID to lower-case.
        id = src.PopFront();
        StringLower(id);

        // Perform any remaining processing of this line, e.g flags.
        if (!processFlags()) {
            src.SkipBlock();
            continue;
        }

        // Process lines that follow after this.
        if (!consumeLines()) {
            src.SkipBlock();
            continue;
        }

		finishBlock();
    }
}

bool
FileParser::consumeLines()
{
	// The base implementation is pretty naive - just tokenize lines and try
	// to consume them.
    while (src.GetLineTerms()) {
        CheckErrorCount();
        if (!consumeLine())
            return false;
	}
    return true;
}