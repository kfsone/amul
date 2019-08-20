# Option for enabling CLANG_TIDY

OPTION(AMUL_CLANG_TIDY "Use clang-tidy on AMUL source code" OFF)
OPTION(AMUL_CLANG_TIDY_FIX "Use -fix-errors with clang-tidy" OFF)
IF (AMUL_CLANG_TIDY)
    IF (AMUL_CLANG_TIDY_FIX)
        SET(CLANG_TIDY_FIX_ERRORS -fix-errors)
    ENDIF ()
    SET(CMAKE_CXX_CLANG_TIDY clang-tidy --checks= --format-style=file ${CLANG_TIDY_FIX_ERRORS} -p=${CMAKE_BINARY_DIR})
	MESSAGE(STATUS "AMUL_CLANG_TIDY is ON")
ENDIF()
