#include <gtest/gtest.h>
#include "gtest_aliases.h"

#include "h/filesystem.h"
#include "h/filesystem.inl.h"
#include "h/sourcefile.h"

TEST(FilesystemTest, PathCopy)
{
    char into[64] = {0};
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "abcdefg"));
    EXPECT_STREQ("abcdefg", into);
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "/abcdefg"));
    EXPECT_STREQ("/abcdefg", into);
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "\\abcdefg"));
    EXPECT_STREQ("/abcdefg", into);
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), NULL, "///\\\\abcdefg"));
    EXPECT_STREQ("/abcdefg", into);
}

TEST(FilesystemTest, PathCopyOffset)
{
    char   into[64] = {0};
    size_t offset = 0;
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), &offset, "///\\\\abcdefg"));
    EXPECT_STREQ("/abcdefg", into);
    EXPECT_EQ(8, offset);

    offset = 2;
    EXPECT_SUCCESS(PathCopy(into, sizeof(into), &offset, "fred"));
    EXPECT_EQ(7, offset);
    EXPECT_STREQ("/a/fred", into);

    EXPECT_SUCCESS(PathCopy(into, sizeof(into), &offset, "/"));
    EXPECT_EQ(7, offset);
}

TEST(FilesystemTest, PathJoin)
{
    char into[64];

    EXPECT_SUCCESS(PathJoin(into, sizeof(into), "/\\////a//\\////", "\\//b\\//"));
    EXPECT_STREQ("/a/b", into);
}

TEST(FilesystemTest, PathJoinConstraints)
{
    char into[64];

    // simple constraints: bad parameters
    EXPECT_ERROR(EINVAL, PathJoin(NULL, 0, NULL, NULL));
    EXPECT_ERROR(EINVAL, PathJoin(into, 0, NULL, NULL));
    EXPECT_ERROR(EINVAL, PathJoin(into, sizeof(into), ".", NULL));
    EXPECT_ERROR(EINVAL, PathJoin(into, 2, ".", "a"));
    EXPECT_ERROR(EINVAL, PathJoin(into, 3, ".", "a"));

    EXPECT_SUCCESS(PathJoin(into, sizeof(into), ".", "a"));
    EXPECT_STREQ(into, "./a");
    EXPECT_SUCCESS(PathJoin(into, 4, ".", "a"));
    EXPECT_SUCCESS(PathJoin(into, 4, ".//", "a"));
    EXPECT_SUCCESS(PathJoin(into, 4, ".///////////////////", "a"));
    EXPECT_SUCCESS(PathJoin(into, 4, ".//", "////a"));
    EXPECT_STREQ("./a", into);

    EXPECT_SUCCESS(PathJoin(
            into, sizeof(into), "\\\\////\\\\//\\//a\\//b\\////\\c", "//////d////\\\\\\e//"));
    EXPECT_STREQ(into, "/a/b/c/d/e");
}

TEST(FilesystemTest, PathJoiner)
{
    char filepath[MAX_PATH_LENGTH];
    EXPECT_SUCCESS(path_joiner(filepath, ".", "title.txt"));
    EXPECT_STREQ(filepath, "./title.txt");

    strcpy(gameDir, "c:\\users\\oliver\\\\//");
    EXPECT_SUCCESS(gamedir_joiner("\\precious\\rooms.txt"));
    EXPECT_STREQ(filepath, "c:/users/oliver/precious/rooms.txt");
    gameDir[0] = 0;
}

TEST(FilesystemTest, GetFileSizeChecks)
{
    const char *datafile = "getfilesize_test_file1.txt";
    EXPECT_ERROR(EINVAL, GetFilesSize(NULL, NULL));
    EXPECT_ERROR(EINVAL, GetFilesSize(datafile, NULL));
    size_t size = 0;
    EXPECT_ERROR(EINVAL, GetFilesSize(NULL, &size));
    EXPECT_EQ(size, 0);
}

TEST(FilesystemTest, GetFileSizeNoSuchFile)
{
    const char *datafile = "getfilesize_test_file1.txt";
    unlink(datafile);
    size_t size = 0;
    EXPECT_ERROR(ENOENT, GetFilesSize(datafile, &size));
    EXPECT_EQ(size, 0);
}

TEST(FilesystemTest, GetFileSizeNewFile)
{
    const char *datafile = "getfilesize_test_file1.txt";
    FILE *      fp = fopen(datafile, "w");
    EXPECT_NOT_NULL(fp);
    fprintf(fp, "%s", datafile);
    fclose(fp);

    size_t size = 0;
    EXPECT_SUCCESS(GetFilesSize(datafile, &size));
    EXPECT_EQ(size, strlen(datafile));

    unlink(datafile);
}

TEST(FilesystemTest, MakeTestFileNameChecks)
{
    char filepath[MAX_PATH_LENGTH]{};

    EXPECT_ERROR(EINVAL, MakeTextFileName(nullptr, filepath));
    EXPECT_STREQ("", filepath);

    // When gamedir is actually empty, it's invalid to append a path
    EXPECT_STREQ("", gameDir);
    EXPECT_ERROR(EINVAL, MakeTextFileName("/a/b/c", filepath));
    EXPECT_STREQ(filepath, "");
}

TEST(FilesystemTest, MakeTestFileName)
{
    char filepath[MAX_PATH_LENGTH]{0};

    strcpy(gameDir, ".");
    EXPECT_ERROR(EINVAL, MakeTextFileName(nullptr, filepath));
    EXPECT_STREQ(filepath, "");
    EXPECT_SUCCESS(MakeTextFileName("/a/b/c", filepath));
    EXPECT_STREQ(filepath, "./a/b/c.txt");
    gameDir[0] = 0;
}

TEST(FilesystemTest, MakeTestFileNameRoot)
{
    char filepath[MAX_PATH_LENGTH]{0};

    strcpy(gameDir, "/");
    EXPECT_SUCCESS(MakeTextFileName("a/b/c", filepath));
    EXPECT_STREQ(filepath, "/a/b/c.txt");
    gameDir[0] = 0;
}

TEST(FilesystemTest, SourceFileCtor)
{
    SourceFile sf{""};
    EXPECT_EQ(sf.filepath, "");
    EXPECT_NULL(sf.mapping);
    EXPECT_EQ(sf.lineNo, 0);
    EXPECT_EQ(sf.size, 0);
    EXPECT_NULL(sf.buffer.begin());
}

TEST(FilesystemTest, SourceFileOpenEmpty)
{
    const char *txtfile = "sourcefile_test_file1.txt";
    unlink(txtfile);
    FILE *fp = fopen(txtfile, "w");
    EXPECT_NOT_NULL(fp);
    fclose(fp);

    // Check for an empty file returning ENODATA.
    SourceFile sf{txtfile};
    EXPECT_EQ(sf.filepath, txtfile);
    EXPECT_ERROR(ENODATA, sf.Open());
    EXPECT_NULL(sf.mapping);
    EXPECT_NULL(sf.buffer.begin());

    unlink(txtfile);
}

TEST(FilesystemTest, SourceFile)
{
    const char *txtfile = "sourcefile_test_file1.txt";
    unlink(txtfile);
    const char *test1 = "Test 1.";
    const char *test2 = "Test 2.";
    FILE *      fp = fopen(txtfile, "w");
    EXPECT_NOT_NULL(fp);
    fprintf(fp, "%s %s", test1, test2);
    fclose(fp);

    SourceFile sf{txtfile};
    EXPECT_EQ(sf.filepath, txtfile);
    EXPECT_NULL(sf.mapping);
    EXPECT_TRUE(sf.Eof());

    EXPECT_SUCCESS(sf.Open());
    EXPECT_NOT_NULL(sf.mapping);
    EXPECT_NOT_NULL(sf.buffer.begin());
    EXPECT_EQ(0, sf.lineNo);
    EXPECT_EQ(strlen(test1) + 1 + strlen(test1), sf.size);
    EXPECT_EQ(sf.buffer.begin(), sf.mapping);
    EXPECT_EQ(sf.buffer.begin(), sf.buffer.it());
    EXPECT_EQ(sf.size, sf.buffer.end() - sf.buffer.begin());

    EXPECT_STREQ((const char *)sf.mapping, "Test 1. Test 2.");

    sf.Close();
    EXPECT_NULL(sf.mapping);
    EXPECT_NULL(sf.buffer.begin());

    unlink(txtfile);
    size_t size{0};
    EXPECT_ERROR(ENOENT, GetFilesSize(txtfile, &size));
}
