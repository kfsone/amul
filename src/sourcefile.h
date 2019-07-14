#ifndef AMUL_SRC_SOURCEFILE_H
#define AMUL_SRC_SOURCEFILE_H

struct SourceFile {
    char           filepath[MAX_PATH_LENGTH];
    void *         mapping;
    struct Buffer *buffer;
    uint16_t       lineNo;
    size_t         size;
};

#endif