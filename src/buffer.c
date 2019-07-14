#include "buffer.h"

#include <memory.h>

#include <h/amul.alog.h>
#include <h/amul.test.h>

enum { NUM_BUFFERS = 16 };
struct Buffer s_buffer[NUM_BUFFERS];
bool s_bufferInUse[NUM_BUFFERS];

error_t
NewBuffer(const char *data, const size_t dataSize, struct Buffer **receiver)
{
    REQUIRE(data && receiver);
    REQUIRE(*receiver == NULL);

    // Protect against address overflow
    REQUIRE(data + dataSize >= data);

    for (size_t i = 0; i < NUM_BUFFERS; ++i) {
        if (!s_bufferInUse[i]) {  // TODO: atomic swap
            s_bufferInUse[i] = true;
            *receiver = &s_buffer[i];
            break;
        }
    }
    if (*receiver == NULL) {
        alog(AL_WARN, "Had to allocate buffer memory");

        *receiver = malloc(sizeof(struct Buffer));
        CHECK_ALLOCATION(receiver);
    }

    (*receiver)->start = data;
    (*receiver)->end = data + dataSize;
    (*receiver)->pos = data;

    return 0;
}

void
CloseBuffer(struct Buffer **holder)
{
    if (!holder || !*holder)
        return;
    if (*holder >= &s_buffer[0] && *holder <= &s_buffer[NUM_BUFFERS]) {
        size_t bufferNo = *holder - &s_buffer[0];
        if (s_bufferInUse[bufferNo] == false) {
            alog(AL_FATAL, "Double-free of buffer attempted");
        }
        s_bufferInUse[bufferNo] = false;
    } else {
        free(*holder);
    }
    *holder = NULL;
}
