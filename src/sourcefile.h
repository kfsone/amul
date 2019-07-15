#ifndef AMUL_SRC_SOURCEFILE_H
#define AMUL_SRC_SOURCEFILE_H

#include <h/amul.type.h>
#include <h/amul.enum.h>

struct SourceFile {
    char           filepath[MAX_PATH_LENGTH];
    void *         mapping;
    struct Buffer *buffer;
    uint16_t       lineNo;
    size_t         size;
};

extern error_t NewSourceFile(const char *filename, struct SourceFile **sourcefilep);
extern void CloseSourceFile(struct SourceFile **sourcefilep);

#endif