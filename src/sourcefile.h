#ifndef AMUL_SRC_SOURCEFILE_H
#define AMUL_SRC_SOURCEFILE_H

#include <h/amul.type.h>
#include <h/amul.enum.h>

struct SourceFile {
    char           filepath[MAX_PATH_LENGTH];
    void *         mapping { nullptr };
    struct Buffer *buffer { nullptr };
    uint16_t       lineNo { 0 };
    size_t         size { 0 };
};

extern error_t NewSourceFile(const char *filename, struct SourceFile **sourcefilep);
extern void CloseSourceFile(struct SourceFile **sourcefilep);

#endif  // AMUL_SRC_SOURCEFILE_H
