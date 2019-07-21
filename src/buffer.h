#ifndef AMUL_SRC_BUFFER_H
#define AMUL_SRC_BUFFER_H

// Buffer is for consuming bytes from a fixed range in memory.

#include <h/amul.type.h>

struct Buffer {
    const char *pos;
    const char *end;
    const char *start;
};

// Formal methods

// Attempts to use a static buffer or allocates one instead.
extern error_t NewBuffer(const char *data, const size_t dataSize, struct Buffer **receiver);
// Release a buffer
extern void CloseBuffer(struct Buffer **bufferp);

// Inline helpers
static inline size_t
BufferSize(const struct Buffer *buffer)
{
    return buffer->end - buffer->start;
}

static inline bool
BufferEOF(const struct Buffer *buffer)
{
    return (buffer->pos >= buffer->end);
}

static inline char
BufferPeek(struct Buffer *buffer)
{
    if (BufferEOF(buffer))
        return 0;
    while (*buffer->pos < 10) {
        ++buffer->pos;
        if (BufferEOF(buffer))
            return 0;
	}
    return *buffer->pos;
}

static inline char
BufferNext(struct Buffer *buffer)
{
    char c = BufferPeek(buffer);
    if (c)
        ++buffer->pos;
    return c;
}

static inline char
BufferSkip(struct Buffer *buffer)
{
    if (!BufferEOF(buffer))
        return *(++(buffer->pos));
    return 0;
}

#endif  // AMUL_SRC_BUFFER_H
