#include "filesystem.h"
#include "sourcefile.h"
#include <gtest/gtest.h>
#include "gtest_aliases.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

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

    char gameDir[MAX_PATH_LENGTH] = "c:\\users\\oliver\\\\//";
    EXPECT_SUCCESS(gamedir_joiner("\\precious\\rooms.txt"));
	EXPECT_STREQ(filepath, "c:/users/oliver/precious/rooms.txt");
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

TEST(FilesystemTest, FileMappingChecks)
{
    // all of these should produce contract failures
    void *data = NULL;
    EXPECT_ERROR(EINVAL, NewFileMapping(NULL, NULL, 1));
    EXPECT_ERROR(EINVAL, NewFileMapping("", NULL, 1));
    EXPECT_ERROR(EINVAL, NewFileMapping("a", NULL, 1));
    EXPECT_ERROR(EINVAL, NewFileMapping("a", &data, 0));
    data = (void*)(uintptr_t)0xdeadbeef;
    EXPECT_ERROR(EINVAL, NewFileMapping("a", &data, 1));
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
    size_t size = 0;
    EXPECT_SUCCESS(GetFilesSize(datafile, &size));
    EXPECT_EQ(expectedSize, size);

    // with everything ready, make sure passing a 0 size fails.
    void *data = NULL;
    EXPECT_ERROR(EINVAL, NewFileMapping(datafile, &data, 0));
    EXPECT_NULL(data);

    EXPECT_SUCCESS(NewFileMapping(datafile, &data, size));
    EXPECT_NOT_NULL(data);
    EXPECT_SUCCESS(strncmp(first, (const char*)data, strlen(first)));
    unlink(datafile);

    EXPECT_SUCCESS(strncmp(first, (const char*)data, strlen(first)));
    CloseFileMapping(&data, size);
    EXPECT_NULL(data);
}

extern error_t makeTextFileName(struct SourceFile *, const char*);

TEST(FilesystemTest, MakeTestFileNameChecks)
{
    SourceFile sf = {{0}};
    EXPECT_ERROR(EINVAL, makeTextFileName(NULL, NULL));
    EXPECT_ERROR(EINVAL, makeTextFileName(NULL, "a"));
    EXPECT_ERROR(EINVAL, makeTextFileName(&sf, NULL));

	EXPECT_STREQ("", gameDir);
	EXPECT_STREQ("", sf.filepath);
}

TEST(FilesystemTest, MakeTestFileName)
{
    SourceFile sf = {{0}};
    EXPECT_ERROR(EINVAL, makeTextFileName(&sf, "/a/b/c"));
	EXPECT_STREQ(sf.filepath, "");
    strcpy(gameDir, ".");
    EXPECT_ERROR(EINVAL, makeTextFileName(&sf, NULL));
	EXPECT_STREQ("", sf.filepath);
    EXPECT_SUCCESS(makeTextFileName(&sf, "/a/b/c"));
	EXPECT_STREQ(sf.filepath, "./a/b/c.txt");

    gameDir[0] = 0;
}

TEST(FilesystemTest, MakeTestFileNameRoot)
{
    SourceFile sf = {{0}};
	strcpy(gameDir, "/");
    EXPECT_SUCCESS(makeTextFileName(&sf, "a/b/c"));
    EXPECT_STREQ(sf.filepath, "/a/b/c.txt");
	gameDir[0] = 0;
}

extern SourceFile s_sourceFile;
extern bool s_sourceFileInUse;

TEST(FilesystemTest, SourceFileChecks)
{
    SourceFile *sourcefile{nullptr};
    size_t      size = 0;
    const char *filename = "sourcefile_test_file1";
    const char *txtfile = "./sourcefile_test_file1.txt";
    unlink(txtfile);
    
	EXPECT_FALSE(s_sourceFileInUse);

	EXPECT_ERROR(EINVAL, NewSourceFile(NULL, &sourcefile));
    EXPECT_ERROR(EINVAL, NewSourceFile("a", NULL));

    s_sourceFileInUse = true;
    EXPECT_ERROR(ENFILE, NewSourceFile("a", &sourcefile));
    s_sourceFileInUse = false;

	EXPECT_STREQ(gameDir, "");
    EXPECT_ERROR(EINVAL, NewSourceFile(filename, &sourcefile));
    EXPECT_NULL(s_sourceFile.buffer.Start());

    strcpy(gameDir, ".");
    EXPECT_ERROR(ENOENT, GetFilesSize(txtfile, &size));
    EXPECT_ERROR(ENOENT, NewSourceFile(filename, &sourcefile));
    EXPECT_STREQ(txtfile, s_sourceFile.filepath);
    EXPECT_NULL(s_sourceFile.buffer.Start());
}

TEST(FilesystemTest, SourceFileNoData)
{
    SourceFile *sourcefile{nullptr};
    size_t      size = 0;
    const char *filename = "sourcefile_test_file1";
    const char *txtfile = "./sourcefile_test_file1.txt";
    unlink(txtfile);

    // Check for an empty file returning ENODATA.
    FILE *fp = fopen(txtfile, "w");
    EXPECT_NOT_NULL(fp);
    fclose(fp);
    EXPECT_ERROR(ENODATA, NewSourceFile(filename, &sourcefile));
    EXPECT_FALSE(s_sourceFileInUse);
    EXPECT_NULL(s_sourceFile.buffer.Start());

	unlink(txtfile);
}

TEST(FilesystemTest, SourceFile)
{
    SourceFile *sourcefile{nullptr};
	size_t      size = 0;
    const char *filename = "sourcefile_test_file1";
    const char *txtfile = "./sourcefile_test_file1.txt";
    unlink(txtfile);

    // Now make the file bigger and check we get it mapped.
    const char *test1 = "Test 1.";
    const char *test2 = "Test 2.";
    FILE *fp = fopen(txtfile, "w");
    EXPECT_NOT_NULL(fp);
    fprintf(fp, "%s %s", test1, test2);
    fclose(fp);
    EXPECT_SUCCESS(NewSourceFile(filename, &sourcefile));
    EXPECT_EQ(&s_sourceFile, sourcefile);
    EXPECT_TRUE(s_sourceFileInUse);
    EXPECT_STREQ(sourcefile->filepath, txtfile);
    EXPECT_NOT_NULL(sourcefile->mapping);
    EXPECT_NOT_NULL(sourcefile->buffer.Start());
    EXPECT_EQ(0, sourcefile->lineNo);
    EXPECT_EQ(strlen(test1) + 1 + strlen(test1), sourcefile->size);
    EXPECT_EQ(sourcefile->buffer.Start(), sourcefile->mapping);
    EXPECT_EQ(sourcefile->buffer.Start(), sourcefile->buffer.Pos());
    EXPECT_EQ(sourcefile->size, sourcefile->buffer.End() - sourcefile->buffer.Start());

	EXPECT_STREQ((const char*)sourcefile->mapping, "Test 1. Test 2.");

    // If we try to re-open the source file we should get a ENFILE
    EXPECT_ERROR(ENFILE, NewSourceFile(filename, &sourcefile));
    EXPECT_NOT_NULL(sourcefile);

	CloseSourceFile(&sourcefile);
    EXPECT_NULL(sourcefile);

    unlink(txtfile);
    EXPECT_ERROR(ENOENT, GetFilesSize(txtfile, &size));
}

TEST(FilesystemTest, ClearGameDir)
{
    gameDir[0] = 0;
}

