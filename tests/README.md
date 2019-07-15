# kftest1

Homebrew unit-testing system, because I wasn't finding something I liked
in the generally available C community.

The name is based on my gamertag (KFS1/kfsone).


# Testing

Build the `tests` target to produce the `tests` or `tests.exe` executable
and run it.

If any tests fail, it will exit with a non-zero status. From inside a
debugger, it should attempt to use a break point if the compiler/debugger
support it.


# Command line

There aren't any command line features yet.


# Developing

## Integration

1. Write your tests,
2. Write a harness function that calls them using `RUN_TEST(fn)`,
3. Register your suite function in `testmain.c` near the top,
4. Add your suite to the `suites` list,
5. Add the relevant source files / libraries to CMakeLists.txt

## Writing Tests

My personal convention, by no means required:

1. For a file called "module.c" I create "module_tests.c"
2. For each function I'm testing, I create a test_function_name
3. In "module_tests.c" I have a function called module_suite
4. module_suite uses RUN_TEST(...) to invoke each test in the suite

