struct Vector {
    size_t growSize;
    void * ptr;
    size_t size;
    size_t blocks;
};

error_t
VectorPushBytes(struct Vector *vector, const void *start, const void *end, size_t *lengthp)
{
    REQUIRE(vector && start);
    REQUIRE(!end || end >= start);
    if (end == NULL) {
        while (*(const char *)end) {
            end = (const char *)end + 1;
        }
    }
    // include the \0
    size_t length = (const char *)end - (const char *)start + 1;
    if (lengthp)
        *lengthp = length;

    size_t newSize = vector->size + length;
    size_t newBlocks = newSize / vector->growSize;
    if (newBlocks >= vector->blocks) {
        size_t newCapacity = newBlocks * vector->growSize;
        void * newPtr = realloc(vector->ptr, newCapacity);
        CHECK_ALLOCATION(newPtr);
        vector->ptr = newPtr;
        vector->blocks = newBlocks;
    }
}
