////////////////////////////////////////////////////////////////////////////////////////////////////
// kf1test : kfsone's homebrew C unit testing system
//
// See README.md for instructions

#include <h/amul.alog.h>
#include <h/amul.test.h>
#include "testing.h"

#include <string>
#include <vector>

static TestCase *m_cases;


// Self registration
TestCase::TestCase(const char *suite, const char *name, TestHandler fn)
	: m_suite{suite}, m_name{name}, m_handler{fn}, m_next{nullptr}
{
	TestCase **curp = &m_cases;
	while (*curp) {
		curp = &((*curp)->m_next);
	}
	*curp = this;
}


int
main(int argc, const char **argv)
{
    struct TestContext context = {argc, argv, nullptr, false, 0, 0, 0};

	const char* suite = nullptr;
	TestCase *cur = m_cases;
	for (;cur;) {
        // capture the current pass count so we can delta later.
        size_t passes = context.passes;

		suite = cur->m_suite;
		std::string suitename = suite;
		suitename += "................";
		suitename = suitename.substr(0, 16) + ": ";
		printf("%s", suitename.c_str());
        fflush(stdout);

		do {
			context.tests++;
			context.step = cur->m_name;
			if (context.tearUp) {
				context.tearUp(context);
			}
			cur->m_handler(context);
			if (context.tearDown) {
				context.tearDown(context);
			}
			context.passes++;
			cur = cur->m_next;
		} while (cur && cur->m_suite == suite);

        printf("OK (%zu passes)\n", context.passes - passes);
        fflush(stdout);

        context.userData = nullptr;
        context.tearUp = nullptr;
        context.tearDown = nullptr;
    }

    printf("SUCCESS: %zu/%zu tests passed, %zu evaluations\n", context.passes, context.tests,
           context.lineItems);

    return 0;
}
