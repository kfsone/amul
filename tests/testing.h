#ifndef AMUL_TESTS_TESTING_H
#define AMUL_TESTS_TESTING_H

using TestHandler = void(*)(struct TestContext &t);

// TestContext is passed to every test suite to provide them with the testing environment, such
// as command line, number of tests, etc, as well as a means to share data between tests
// (userData) and to provide tearUp/tearDown functions for tests.

struct TestContext {
    int          argc;
    const char **argv;
    const char * step;
    bool         verbose;
    size_t       tests;
    size_t       passes;
    size_t       lineItems;

    void         *userData;
	TestHandler	 tearUp;
	TestHandler	 tearDown;
};

struct TestCase {
	using TestImpl = void(*)(TestContext &);

	const char	*m_suite;
	const char  *m_name;
	TestHandler m_handler;
	TestCase    *m_next;

	TestCase(const char *suite, const char *name, TestHandler handler);

	void operator()(TestContext &t) {
		m_handler(t);
	}
};

///TODO: We don't need to use macros anymore.

///////////////////////////////////////////////////////////////////////////////////////////////////
// Unit testing macros
//
// These only have any effect with NDEBUG is not defined, the same convention that assert.h uses.

#ifndef NDEBUG

#    define LPRINTF(fmt, ...)                                                                      \
        fprintf(stderr, "\n%s:%d: error:%s: " fmt "\n", __FILE__, __LINE__, t.step,               \
                ##__VA_ARGS__);                                                                    \
        fflush(stderr)

#    define RUN_TEST(fn)                                                                           \
        do {                                                                                       \
            t.tests++;                                                                            \
            t.step = #fn;                                                                         \
            if (t.tearUp)                                                                         \
                t.tearUp(t);                                                                      \
            fn(t);                                                                                 \
            if (t.tearDown)                                                                       \
                t.tearDown(t);                                                                    \
            t.passes++;                                                                           \
        } while (false)

#    define EXPECT_ERROR(expected, actual)                                                         \
        if ((expected) == (actual)) {                                                              \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting error#%d; got %d", (expected), (actual));                  \
            assert((expected) == (actual));                                                        \
        }

#    define EXPECT_SUCCESS(actual)                                                                 \
        if ((0) == (actual)) {                                                                     \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting SUCCESS; got error#%d", (actual));                         \
            assert(0 == (actual));                                                                 \
        }

#    define EXPECT_ZERO(actual)                                                                    \
        if ((uintptr_t)(0) == (uintptr_t)(actual)) {                                               \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting ZERO; got #%" PRid64 "", (int64_t)(actual));               \
            assert(0 == (actual));                                                                 \
        }

#    define EXPECT_VAL_EQUAL(expected, actual)                                                     \
        if ((expected) == (actual)) {                                                              \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting %" PRId64 "; got %" PRId64, (int64_t)(expected),           \
                    (int64_t)(actual));                                                            \
            assert((expected) == (actual));                                                        \
        }

#    define EXPECT_PTR_EQUAL(expected, actual)                                                     \
        if ((void *)(expected) == (void *)(actual)) {                                              \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting %p; got %p", (void *)(expected), (void *)(actual));        \
            assert((void *)(expected) == (void *)(actual));                                        \
        }

#    define EXPECT_NOT_NULL(actual)                                                                \
        if ((actual)) {                                                                            \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting !NULL; got NULL");                                         \
            assert((actual));                                                                      \
        }

#    define EXPECT_NULL(actual)                                                                    \
        if (!(actual)) {                                                                           \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting NULL; got %p", (void *)(actual));                          \
            assert((actual));                                                                      \
        }

#    define EXPECT_TRUE(actual)                                                                    \
        if ((actual)) {                                                                            \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting true; got false");                                         \
            assert((actual));                                                                      \
        }

#    define EXPECT_FALSE(actual)                                                                   \
        if (!(actual)) {                                                                           \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting false; got %" PRId64, (int64_t)(actual));                  \
            assert(!(actual));                                                                     \
        }

#    define EXPECT_STR_EQUAL(expected, actual)                                                     \
        if (strcmp(expected, actual) == 0) {                                                       \
            t.lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting '%s'; got '%s'", (expected), (actual));                    \
            assert(strcmp(expected, actual) == 0);                                                 \
        }

#else

#    define EXPECT_ERROR(...)
#    define EXPECT_SUCCESS(...)
#    define EXPECT_ZERO(...)
#    define EXPECT_VAL_EQUAL(...)
#    define EXPECT_PTR_EQUAL(...)
#    define EXPECT_NOT_NULL(...)
#    define EXPECT_NULL(...)
#    define EXPECT_TRUE(...)
#    define EXPECT_FALSE(...)
#    define EXPECT_STR_EQUAL(...)

#endif


#endif
