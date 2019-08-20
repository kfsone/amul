#include "gtest_aliases.h"
#include <gtest/gtest.h>

#include "filesystem.h"
#include "filesystem.inl.h"
#include "sourcefile.h"

TEST(FilesystemTest, PathAdd)
{
    std::string into{};

    into.clear();
    PathAdd(into, "abcdefg");
    EXPECT_EQ("abcdefg", into);

    into.clear();
    PathAdd(into, "/abcdefg");
    EXPECT_EQ("/abcdefg", into);

    into.clear();
    PathAdd(into, "\\abcdefg");
    EXPECT_EQ("/abcdefg", into);

    into.clear();
    PathAdd(into, "///\\\\abcdefg");
    EXPECT_EQ("/abcdefg", into);

    into.clear();
    PathAdd(into, "foo");
    PathAdd(into, "bar");
    PathAdd(into, "/foo");
    PathAdd(into, "/bar/");
    EXPECT_EQ("foo/bar/foo/bar", into);
}

TEST(FilesystemTest, PathJoin)
{
    std::string filepath{};

    gameDir.clear();
    PathAdd(gameDir, "c:\\users\\oliver\\\\//");
    EXPECT_EQ(gameDir, "c:/users/oliver");

    filepath.clear();
    PathJoin(filepath, "/\\////a//\\////", "\\//b\\//");
    EXPECT_EQ("/a/b", filepath);

    filepath.clear();
    PathJoin(filepath, ".", "title.txt");
    EXPECT_EQ(filepath, "./title.txt");

    EXPECT_SUCCESS(gamedir_joiner("\\precious\\rooms.txt"));
    EXPECT_EQ(filepath, "c:/users/oliver/precious/rooms.txt");
    gameDir.clear();
}

TEST(FilesystemTest, GetFileSizeChecks)
{
    const char *datafile = "getfilesize_test_file1.txt";
    EXPECT_ERROR(EINVAL, GetFilesSize("", nullptr));
    EXPECT_ERROR(EINVAL, GetFilesSize(datafile, nullptr));
    size_t size = 0;
    EXPECT_ERROR(EINVAL, GetFilesSize("", &size));
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
    FILE *fp = fopen(datafile, "w");
    EXPECT_NOT_NULL(fp);
    fprintf(fp, "%s", datafile);
    fclose(fp);

    size_t size = 0;
    EXPECT_SUCCESS(GetFilesSize(datafile, &size));
    EXPECT_EQ(size, strlen(datafile));

    unlink(datafile);
}

TEST(FilesystemTest, MakeTextFileNameChecks)
{
    std::string filepath{};
    EXPECT_ERROR(EINVAL, MakeTextFileName("", filepath));
    EXPECT_EMPTY(filepath);

    // When gamedir is actually empty, it's invalid to append a path
    EXPECT_EMPTY(gameDir);
    EXPECT_ERROR(EINVAL, MakeTextFileName("/a/b/c", filepath));
    EXPECT_EMPTY(filepath);

    gameDir = ".";
    EXPECT_ERROR(EINVAL, MakeTextFileName("", filepath));
    EXPECT_EMPTY(filepath);
    gameDir.clear();
}

TEST(FilesystemTest, MakeTextFileName)
{
    std::string filepath{};

    gameDir = ".";
    EXPECT_SUCCESS(MakeTextFileName("/a/b/c", filepath));
    EXPECT_EQ(filepath, "./a/b/c.txt");
    gameDir.clear();
}

TEST(FilesystemTest, MakeTextFileNameRoot)
{
    std::string filepath{};

    gameDir = "/";
    EXPECT_SUCCESS(MakeTextFileName("a/b/c", filepath));
    EXPECT_EQ(filepath, "/a/b/c.txt");
    gameDir.clear();
}

TEST(FilesystemTest, SourceFileCtor)
{
    SourceFile sf{ "" };
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
    SourceFile sf{ txtfile };
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
    FILE *fp = fopen(txtfile, "w");
    EXPECT_NOT_NULL(fp);
    fprintf(fp, "%s %s", test1, test2);
    fclose(fp);

    SourceFile sf{ txtfile };
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

    EXPECT_STREQ((const char *) sf.mapping, "Test 1. Test 2.");

    sf.Close();
    EXPECT_NULL(sf.mapping);
    EXPECT_NULL(sf.buffer.begin());

    unlink(txtfile);
    size_t size{ 0 };
    EXPECT_ERROR(ENOENT, GetFilesSize(txtfile, &size));
}