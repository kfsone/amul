#pragma once
#ifndef AMULCOM_FILEPARSER_H
#define AMULCOM_FILEPARSER_H

#include "typedefs.h"
#include "sourcefile.h"

// FileParser is the base class for AmulCom file parsing. It expects the
// implementer to provide 'idPrefix()', 'consumeFirstLine(id)', and
// 'consumeRemainingLines()', which will be invoked from "Parse".
class FileParser
{
  protected:
    SourceFile src;
    std::string id;  // current id

	virtual void startBlock() {}  // tear-up any state at the start of a block
    virtual string_view idPrefix() const noexcept = 0;
    virtual bool processFlags() = 0;
    virtual bool consumeLines();	// default implementation: consumeLine() on remaining lines
    virtual bool consumeLine() = 0;
    virtual void finishBlock() {}  // called after a block has been successfully consumed

  public:
    FileParser(const std::string &filepath) : src(filepath) {}
	virtual ~FileParser() {}

    template<typename... Args>
    auto Error(Args &&... args) {
        LogError(src.filepath, ":", src.lineNo, ":", id, ": ", std::forward<Args>(args)...);
    }

    void Parse();
};

#endif  // AMULCOM_FILEPARSER_H

