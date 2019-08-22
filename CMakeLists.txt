# CMakeList.txt : CMake project for AMUL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

PROJECT(AMUL)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(AMUL_LIB_NAME "amullib")
SET(AMUL_COMPILER_NAME "amulcom")
SET(AMUL_ENGINE_NAME "amul")

FILE(GLOB AMUL_HEADERS ${PROJECT_SOURCE_DIR}/h/amul.*.h)
FILE(GLOB AMUL_SRC_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/h" "${CMAKE_BINARY_DIR}")

INCLUDE(h/PlatformChecks.cmake)

STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " ${AMUL_FLAGS_DEBUG}")
STRING(APPEND CMAKE_CXX_FLAGS " ${AMUL_FLAGS}")

INCLUDE(h/ClangTidy.cmake)
#INCLUDE(h/CPPCheck.cmake)
INCLUDE(h/Sanitizers.cmake)
INCLUDE(h/UnitTests.cmake)
INCLUDE(h/Ctags.cmake)

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
