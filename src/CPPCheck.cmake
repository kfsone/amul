OPTION(AMUL_CPPCHECK "Use CPP Check to validate code (if available)" OFF)
IF (AMUL_CPPCHECK)
    FIND_PROGRAM(CPPCHECK_EXE NAMES cppcheck)
    IF (NOT CPPCHECK_EXE STREQUAL "CPPCHECK_EXE-NOTFOUND")
		MESSAGE(STATUS "AMUL_CPPCHECK is ON")
		SET(CPPCHECK_ARGS "--enable=all;--template={file}:{line}: {severity}: {id} {message} {callstack};--quiet")
        SET(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE};${CPPCHECK_ARGS}")
    ENDIF()
ENDIF ()