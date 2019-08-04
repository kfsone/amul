/*
 * miscellaneous routines - stop putting them in random places, oliver
 */
#include "errors.hpp"
#include "smuglcom.hpp"

/* Extend an existing, or allocate a new, area of memory, for a given
** purpose (described by msg). A wrapper for malloc/realloc */
void *
grow(void *ptr, size_t size, const char *msg)
{
    void *p;

    if (ptr == nullptr) {
        p = malloc(size);
        if (p == nullptr) {
            error("Out of memory: %s\n", msg);
            errabort();
        }
        return p;
    }

    p = realloc(ptr, size);
    if (p == nullptr) {
        error("Can't extend memory: %s\n", msg);
        errabort();
    }
    return p;
}
