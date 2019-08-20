# CMakeList.txt : CMake project for AMUL, include source and define
# project specific logic here.
#
CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

PROJECT(AMUL CXX C)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(AMUL_LIB_NAME "amullib")
SET(AMUL_COMPILER_NAME "amulcom")
SET(AMUL_ENGINE_NAME "amul")
SET(CONFIG_H_IN "${CMAKE_SOURCE_DIR}/src/amulconfig.h.in")
SET(CONFIG_H "${CMAKE_BINARY_DIR}/amulconfig.h")

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src")
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}")
ADD_COMPILE_DEFINITIONS("HAVE_CONFIG_H=${CONFIG_H}")

FILE(GLOB AMUL_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)

INCLUDE(src/PlatformChecks.cmake)

STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " ${AMUL_FLAGS_DEBUG}")
STRING(APPEND CMAKE_CXX_FLAGS " ${AMUL_FLAGS}")

INCLUDE(src/ClangTidy.cmake)
INCLUDE(src/Sanitizers.cmake)
INCLUDE(src/UnitTests.cmake)
INCLUDE(src/Ctags.cmake)

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
