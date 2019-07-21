////////////////////////////////////////////////////////////////////////////////////////////////////
// kf1test : kfsone's homebrew C unit testing system
//
// See README.md for instructions

#include <h/amul.alog.h>
#include <h/amul.test.h>

testsuite_fn buffer_tests, filesystem_tests, hashmap_tests, modules_tests, tokenizer_tests;

static struct Suite {
    const char *name;
    testsuite_fn *suiteFn;
} suites[] = {

	// Each suite listed here must be registered via the test_suite_fn line above
	{"buffer", 	    buffer_tests},
	{"filesystem",  filesystem_tests},
	{"hashmap", 	hashmap_tests},
	{"modules", 	modules_tests},
    {"tokenizer",   tokenizer_tests},

	{NULL, NULL},
};

int
main(int argc, const char **argv)
{
    struct TestContext context = {argc, argv, NULL, false, 0, 0, 0};

    for (size_t i = 0; suites[i].suiteFn; ++i) {
        // capture the current pass count so we can delta later.
        size_t passes = context.passes;

		char suitename[21];
		snprintf(suitename, sizeof(suitename), "%-16s...:", suites[i].name);
		for (char *p = suitename; *p != ':'; ++p) {
			*p = (*p == ' ') ? '.' : *p;
		}
        printf("%s ", suitename);
        fflush(stdout);

        suites[i].suiteFn(&context);

        printf("OK (%zu passes)\n", context.passes - passes);
        fflush(stdout);

        context.userData = NULL;
        context.tearUp = NULL;
        context.tearDown = NULL;
    }

    printf("SUCCESS: %zu/%zu tests passed, %zu evaluations\n", context.passes, context.tests,
           context.lineItems);

    return 0;
}
