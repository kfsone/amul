#include <h/amul.test.h>
#include <src/buffer.h>
#include <src/filesystem.h>
#include <src/sourcefile.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

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
test_path_joiner(struct TestContext *t)
{
    char filepath[MAX_PATH_LENGTH];
    EXPECT_SUCCESS(path_joiner(filepath, ".", "title.txt"));
    EXPECT_STR_EQUAL(filepath, "./title.txt");

    char gameDir[MAX_PATH_LENGTH] = "c:\\users\\oliver\\\\//";
    EXPECT_SUCCESS(gamedir_joiner("\\precious\\rooms.txt"));
    EXPECT_STR_EQUAL(filepath, "c:/users/oliver/precious/rooms.txt");
}

void
test_get_files_size(struct TestContext *t)
{
    EXPECT_ERROR(EINVAL, GetFilesSize(NULL, NULL));
    EXPECT_ERROR(EINVAL, GetFilesSize(t->argv[0], NULL));
    size_t size = 0;
    EXPECT_SUCCESS(GetFilesSize(t->argv[0], &size));
    EXPECT_FALSE(size == 0);

    const char *datafile = "getfilesize_test_file1.txt";
    unlink(datafile);
    EXPECT_ERROR(ENOENT, GetFilesSize(datafile, &size));

    FILE *fp = fopen(datafile, "w");
    EXPECT_NOT_NULL(fp);
    fclose(fp);

    EXPECT_SUCCESS(GetFilesSize(datafile, &size));
    EXPECT_VAL_EQUAL(0, size);

    unlink(datafile);

    // check nothing gets cached
    EXPECT_ERROR(ENOENT, GetFilesSize(datafile, &size));
}

void
test_file_mapping_checks(struct TestContext *t)
{
    // all of these should produce contract failures
    void *data = NULL;
    EXPECT_ERROR(EINVAL, NewFileMapping(NULL, NULL, 1));
    EXPECT_ERROR(EINVAL, NewFileMapping("", NULL, 1))
    EXPECT_ERROR(EINVAL, NewFileMapping("a", NULL, 1));
    EXPECT_ERROR(EINVAL, NewFileMapping("a", &data, 0));
    data = (void*)(uintptr_t)0xdeadbeef;
    EXPECT_ERROR(EINVAL, NewFileMapping("a", &data, 1));
}

void
test_file_mapping(struct TestContext *t)
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
    EXPECT_VAL_EQUAL(expectedSize, size);

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

void
test_make_test_file_name(struct TestContext *t)
{
    struct SourceFile sf = {{0}};
    EXPECT_ERROR(EINVAL, makeTextFileName(NULL, NULL));
    EXPECT_ERROR(EINVAL, makeTextFileName(NULL, "a"));
    EXPECT_ERROR(EINVAL, makeTextFileName(&sf, NULL));

    EXPECT_STR_EQUAL("", gameDir);
    EXPECT_STR_EQUAL("", sf.filepath);

    EXPECT_ERROR(EINVAL, makeTextFileName(&sf, "/a/b/c"));
    EXPECT_STR_EQUAL("", sf.filepath);
    strcpy(gameDir, ".");
    EXPECT_ERROR(EINVAL, makeTextFileName(&sf, NULL));
    EXPECT_STR_EQUAL("", sf.filepath);
    EXPECT_SUCCESS(makeTextFileName(&sf, "/a/b/c"));
    EXPECT_STR_EQUAL("./a/b/c.txt", sf.filepath);

    gameDir[0] = 0;
}

extern struct SourceFile s_sourceFile;
extern bool s_sourceFileInUse;

void
test_sourcefile(struct TestContext *t)
{
    EXPECT_FALSE(s_sourceFileInUse);

    struct SourceFile* sourcefile = NULL;
    EXPECT_ERROR(EINVAL, NewSourceFile(NULL, &sourcefile));
    EXPECT_ERROR(EINVAL, NewSourceFile("a", NULL));

    s_sourceFileInUse = true;
    EXPECT_ERROR(ENFILE, NewSourceFile("a", &sourcefile));
    s_sourceFileInUse = false;

    size_t size = 0;
    const char *filename = "sourcefile_test_file1";
    const char *txtfile = "./sourcefile_test_file1.txt";
    unlink(txtfile);

    EXPECT_STR_EQUAL(gameDir, "");
    EXPECT_ERROR(EINVAL, NewSourceFile(filename, &sourcefile));
    EXPECT_NULL(s_sourceFile.buffer.Start());

    strcpy(gameDir, ".");
    EXPECT_ERROR(ENOENT, GetFilesSize(txtfile, &size));
    EXPECT_ERROR(ENOENT, NewSourceFile(filename, &sourcefile));
    EXPECT_STR_EQUAL(txtfile, s_sourceFile.filepath);
    EXPECT_NULL(s_sourceFile.buffer.Start());

    // Check for an empty file returning ENODATA.
    FILE *fp = fopen(txtfile, "w");
    EXPECT_NOT_NULL(fp);
    fclose(fp);
    EXPECT_ERROR(ENODATA, NewSourceFile(filename, &sourcefile));
    EXPECT_FALSE(s_sourceFileInUse);
    EXPECT_NULL(s_sourceFile.buffer.Start());

    // Now make the file bigger and check we get it mapped.
    const char *test1 = "Test 1.";
    const char *test2 = "Test 2.";
    fp = fopen(txtfile, "w");
    EXPECT_NOT_NULL(fp);
    fprintf(fp, "%s %s", test1, test2);
    fclose(fp);
    EXPECT_SUCCESS(NewSourceFile(filename, &sourcefile));
    EXPECT_PTR_EQUAL(&s_sourceFile, sourcefile);
    EXPECT_TRUE(s_sourceFileInUse);
    EXPECT_STR_EQUAL(sourcefile->filepath, txtfile);
    EXPECT_NOT_NULL(sourcefile->mapping);
    EXPECT_NOT_NULL(sourcefile->buffer.Start());
    EXPECT_VAL_EQUAL(0, sourcefile->lineNo);
    EXPECT_VAL_EQUAL(strlen(test1) + 1 + strlen(test1), sourcefile->size);
    EXPECT_VAL_EQUAL(sourcefile->buffer.Start(), sourcefile->mapping);
    EXPECT_VAL_EQUAL(sourcefile->buffer.Start(), sourcefile->buffer.Pos());
    EXPECT_VAL_EQUAL(sourcefile->size, sourcefile->buffer.End() - sourcefile->buffer.Start());

    EXPECT_STR_EQUAL((const char*)sourcefile->mapping, "Test 1. Test 2.");

    // If we try to re-open the source file we should get a ENFILE
    EXPECT_ERROR(ENFILE, NewSourceFile(filename, &sourcefile));
    EXPECT_NOT_NULL(sourcefile);

	CloseSourceFile(&sourcefile);
    EXPECT_NULL(sourcefile);

    unlink(txtfile);
    EXPECT_ERROR(ENOENT, GetFilesSize(txtfile, &size));
}

void
clearGameDir(struct TestContext *t)
{
    gameDir[0] = 0;
}

void
filesystem_tests(struct TestContext *t)
{
    t->tearUp = clearGameDir;
    t->tearDown = clearGameDir;

    RUN_TEST(test_path_copy);
    RUN_TEST(test_path_copy_offset);
    RUN_TEST(test_path_join);
    RUN_TEST(test_path_join_constraints);
    RUN_TEST(test_path_joiner);
    RUN_TEST(test_get_files_size);
    RUN_TEST(test_file_mapping_checks);
    RUN_TEST(test_file_mapping);
    RUN_TEST(test_make_test_file_name);
    RUN_TEST(test_sourcefile);
}
