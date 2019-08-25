#include "gtest_aliases.h"
#include <gtest/gtest.h>

#include "h/filemapping.h"
#include "h/filesystem.h"


TEST(FilesystemTest, FileMappingChecks)
{
    // all of these should produce contract failures
    size_t size { 0 };
    void *data { nullptr };
    EXPECT_ERROR(EINVAL, NewFileMapping(nullptr, &data, &size));
    EXPECT_ERROR(EINVAL, NewFileMapping("", &data, &size));
    EXPECT_ERROR(EINVAL, NewFileMapping("a", nullptr, &size));
    data = reinterpret_cast<void*>(0xdeadbeef);
    EXPECT_ERROR(EINVAL, NewFileMapping("a", &data, &size));
}

TEST(FilesystemTest, FileMapping)
{
    const char *datafile = "mapping_test_file1.txt";
    const char *first = "Mapping test.";
    const char *second = "Second sentence.";

    FILE *fp = fopen(datafile, "w");
    EXPECT_NOT_NULL(fp);
    fprintf(fp, "%s %s", first, second);
    fclose(fp);

    size_t expectedSize = strlen(first) + 1 + strlen(second);
    size_t size{0};
    EXPECT_SUCCESS(GetFilesSize(datafile, &size));
    EXPECT_EQ(expectedSize, size);

    void *data = nullptr;
    EXPECT_SUCCESS(NewFileMapping(datafile, &data, &size));
    EXPECT_NOT_NULL(data);
    EXPECT_SUCCESS(strncmp(first, (const char *)data, strlen(first)));
    unlink(datafile);

    EXPECT_SUCCESS(strncmp(first, (const char *)data, strlen(first)));
    CloseFileMapping(&data, size);
    EXPECT_NULL(data);
}
