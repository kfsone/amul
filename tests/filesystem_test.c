#include <h/amul.file.h>
#include <h/amul.test.h>

void
test_path_copy(struct TestContext *t)
{
    char into[64] = {0};
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "abcdefg"));
    EXPECT_STR_EQUAL("abcdefg", into);
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "/abcdefg"));
    EXPECT_STR_EQUAL("/abcdefg", into);
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "\\abcdefg"));
    EXPECT_STR_EQUAL("/abcdefg", into);
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "///\\\\abcdefg"));
    EXPECT_STR_EQUAL("/abcdefg", into);
}

void
test_path_copy_offset(struct TestContext *t)
{
    char   into[64] = {0};
    size_t offset = 0;
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), &offset, "///\\\\abcdefg"));
    EXPECT_STR_EQUAL("/abcdefg", into);
    EXPECT_VAL_EQUAL(8, offset);

    offset = 2;
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), &offset, "fred"));
    EXPECT_VAL_EQUAL(7, offset);
    EXPECT_STR_EQUAL("/a/fred", into);

    EXPECT_SUCCESS(PathCopy(into, sizeof(into), &offset, "/"));
    EXPECT_VAL_EQUAL(7, offset);
}

void
test_path_join(struct TestContext *t)
{
    char into[64];

    EXPECT_SUCCESS(PathJoin(into, sizeof(into), "/\\////a//\\////", "\\//b\\//"));
    EXPECT_STR_EQUAL("/a/b", into);
}

void
test_path_join_constraints(struct TestContext *t)
{
    char into[64];

    // simple constraints: bad parameters
    EXPECT_ERROR(EINVAL, PathJoin(NULL, 0, NULL, NULL));
    EXPECT_ERROR(EINVAL, PathJoin(into, 0, NULL, NULL));
    EXPECT_ERROR(EINVAL, PathJoin(into, sizeof(into), ".", NULL));
    EXPECT_ERROR(EINVAL, PathJoin(into, 2, ".", "a"));
    EXPECT_ERROR(EINVAL, PathJoin(into, 3, ".", "a"));

    EXPECT_SUCCESS(PathJoin(into, sizeof(into), ".", "a"));
    EXPECT_STR_EQUAL(into, "./a");
    EXPECT_SUCCESS(PathJoin(into, 4, ".", "a"));
    EXPECT_SUCCESS(PathJoin(into, 4, ".//", "a"));
    EXPECT_SUCCESS(PathJoin(into, 4, ".///////////////////", "a"));
    EXPECT_SUCCESS(PathJoin(into, 4, ".//", "////a"));
    EXPECT_STR_EQUAL("./a", into);

    EXPECT_SUCCESS(PathJoin(
            into, sizeof(into), "\\\\////\\\\//\\//a\\//b\\////\\c", "//////d////\\\\\\e//"));
    EXPECT_STR_EQUAL(into, "/a/b/c/d/e");
}

void
filesystem_tests(struct TestContext *t)
{
    RUN_TEST(test_path_copy);
    RUN_TEST(test_path_copy_offset);
    RUN_TEST(test_path_join);
    RUN_TEST(test_path_join_constraints);
}
