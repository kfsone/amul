#include <h/amul.test.h>
#include <stdio.h>
#include <stdlib.h>

test_harness_fn hashmap_tests;
test_harness_fn filesystem_tests;
test_harness_fn modules_tests;

void
harness(const char *name, test_harness_fn fn, struct TestContext *t)
{
    size_t passes = t->passes;
    printf("%s...: ", name);
    fflush(stdout);
    fn(t);
    printf("OK (%zu passes)\n", t->passes - passes);
    t->userData = NULL;
    t->tearUp = NULL;
    t->tearDown = NULL;
}

int
main(int argc, const char **argv)
{
    struct TestContext context = {argc, argv, NULL, false, 0, 0, 0};

    harness("hashmap", hashmap_tests, &context);
    harness("filesystem", filesystem_tests, &context);
    harness("modules", modules_tests, &context);

    printf("SUCCESS: %zu/%zu tests passed, %zu evaluations\n", context.passes, context.tests,
           context.lineItems);

    return 0;
}
