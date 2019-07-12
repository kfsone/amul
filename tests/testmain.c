#include <stdlib.h>
#include <stdio.h>

size_t numTests;

typedef void (test_func) (int, const char**);

test_func hashmap_tests;

int main(int argc, const char **argv) {
    hashmap_tests(argc, argv);

    printf("SUCCESS: %zu tests passed\n", numTests);
}
