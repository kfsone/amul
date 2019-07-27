#ifndef AMUL_TESTS_GTEST_ALIASES_H
#define AMUL_TESTS_GTEST_ALIASES_H

#define EXPECT_SUCCESS(predicate) EXPECT_EQ(error_t(0), (predicate))
#define EXPECT_ERROR(err, predicate) EXPECT_EQ(error_t(err), (predicate))
#define EXPECT_NULL(value) EXPECT_EQ(value, nullptr)
#define EXPECT_NOT_NULL(value) EXPECT_NE(value, nullptr)

#endif  // AMUL_TESTS_GTEST_ALIASES_H