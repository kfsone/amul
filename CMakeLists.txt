# CMakeList.txt : CMake project for AMUL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

PROJECT(AMUL CXX)

SET(AMUL_LIB_NAME "amullib")
SET(AMUL_COMPILER_NAME "amulcom")
SET(AMUL_ENGINE_NAME "amul")
FILE(GLOB AMUL_HEADERS ${PROJECT_SOURCE_DIR}/h/amul.*.h)
FILE(GLOB AMUL_SRC_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

OPTION(AMUL_CLANG_TIDY "Use clang-tidy on AMUL source code" ON)
OPTION(AMUL_CLANG_TIDY_FIX "Use -fix-errors with clang-tidy" OFF)
IF (AMUL_CLANG_TIDY)
    IF (AMUL_CLANG_TIDY_FIX)
        SET(CLANG_TIDY_FIX_ERRORS -fix-errors)
    ENDIF ()
    SET(CMAKE_CXX_CLANG_TIDY clang-tidy --checks= --format-style=file ${CLANG_TIDY_FIX_ERRORS} -p=${CMAKE_BINARY_DIR})
ENDIF()

OPTION(AMUL_INCLUDE_TESTS "Build and run unit tests" ON)
IF (AMUL_INCLUDE_TESTS AND NOT AMUL_CLANG_TIDY)
	MESSAGE(STATUS "Testing enabled")
	enable_testing()
	ADD_SUBDIRECTORY(test)
ENDIF()

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")

IF (MSVC)

    add_compile_definitions("_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_DEPRECATE")

ELSE ()

    IF (CMAKE_C_COMPILER_ID MATCHES "[cC][lL][aA][nN][gG]")
        SET(CLANG true)
    ELSEIF (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        SET(GCC true)
    ENDIF ()

    STRING(APPEND AMUL_FLAGS_DEBUG " -O0 -g3")
    STRING(APPEND AMUL_FLAGS " -Wall -Wshadow -Wwrite-strings")
    OPTION(AMUL_SANITIZERS "Enable address/undefined sanitizers" ON)
    IF (AMUL_SANITIZERS)
        STRING(APPEND AMUL_FLAGS_DEBUG " -fsanitize=address -fsanitize=undefined")
        #STRING(APPEND AMUL_FLAGS_DEBUG " -fsanitize=memory")
        #STRING(APPEND AMUL_FLAGS_DEBUG " -fsanitize=safe-stack")
        STRING(APPEND AMUL_FLAGS_DEBUG " -fno-omit-frame-pointer")
        #STRING(APPEND AMUL_FLAGS_DEBUG " -fsanitize-stats")
        IF (CLANG)
            STRING(APPEND AMUL_FLAGS_DEBUG " -fprofile-instr-generate -fcoverage-mapping")
        ENDIF ()
    ENDIF ()

    STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " ${AMUL_FLAGS_DEBUG}")
    STRING(APPEND CMAKE_CXX_FLAGS " ${AMUL_FLAGS}")
ENDIF ()

OPTION(AMUL_CPPCHECK "Use CPP Check to validate code (if available)" OFF)
IF (AMUL_CPPCHECK)
    FIND_PROGRAM(CPPCHECK_EXE NAMES cppcheck)
    IF (NOT CPPCHECK_EXE STREQUAL "CPPCHECK_EXE-NOTFOUND")
        SET(CPPCHECK_ARGS "--enable=all;--template={file}:{line}: {severity}: {id} {message} {callstack};--suppress=missingIncludeSystem;--quiet")
        SET(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE};${CPPCHECK_ARGS}")
    ENDIF()
ENDIF ()

ADD_SUBDIRECTORY(src)

# all:
#   ninja -j4
# 
# .PHONY : force
# force::
#   ninja -j9 clean
#   ninja -j4
# 
# .PHONY : clean
# clean::
#   ninja clean
# 
# .PHONY : tests
# tests::
#   ninja -j4 tests
#   tests/tests
# 
# .PHONE : cov
# cov::
#   ninja clean
#   ninja -j8 tests
#   LLVM_PROFILE_FILE="tests.profraw" tests/tests
#   llvm-profdata-6.0 merge -sparse tests.profraw -o tests.profdata
#   llvm-cov-6.0 show -format=html tests/tests -instr-profile=tests.profdata >cov.html
#   llvm-cov-6.0 report tests/tests -instr-profile=tests.profdata

