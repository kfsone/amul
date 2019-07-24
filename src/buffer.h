#ifndef AMUL_SRC_BUFFER_H
#define AMUL_SRC_BUFFER_H

#include <h/amul.type.h>

///////////////////////////////////////////////////////////////////////////////
// Buffer encapsulates a view onto a range of bytes that has a finite
// start, end and current position. It's primarily designed for text
// (char).
//
// Methods:
//   NewBuffer(dataStart, dataSize, *(Buffer*))
//   	Assigns and initializes a new buffer, to be release via CloseBuffer.
//   	Note: For efficiency, there is a single static buffer so that
//   	compiling file-at-a-time does not require any allocation.
//
//   BufferSize(buffer)
//   	returns the current size in bytes; this is/ currently the same as
//   	the size in characters but may not always be.
//
//   BufferEOF(buffer)
//   	returns true if the cursor (`pos`) has exceeded the current range.
//
//   BufferPeek(buffer)
//   	returns the character at the current cursor without consuming it,
//   	or 0 if the buffer is at EOF
//
//	BufferNext(buffer)	///TODO: rename BufferRead
//		returns the character at the current cursor position and advances
//		the cursor.
//		aka: *(buffer++)
//
//	BufferSkip(buffer)
//		advances the cursor and returns the character at the new position,
//		or 0 on EOF
//		aka: *(++buffer)
// 
struct Buffer {
    const char *pos;
    const char *end;
    const char *start;
};

// Formal methods
extern error_t NewBuffer(const char *data, const size_t dataSize, struct Buffer **receiver);
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
	///TODO: assert buffer->pos >= buffer->start
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
