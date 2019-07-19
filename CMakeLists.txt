CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(AMUL C)

SET(AMUL_LIB_NAME "amullib")
SET(AMUL_COMPILER_NAME "amulcom")
SET(AMUL_ENGINE_NAME "amul")
FILE(GLOB AMUL_HEADERS ${PROJECT_SOURCE_DIR}/h/amul.*.h)

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")

OPTION(AMUL_INCLUDE_TESTS "Build and run unit tests" ON)
IF (AMUL_INCLUDE_TESTS)
	ENABLE_TESTING()
ENDIF ()

IF (MSVC)
    add_compile_definitions("_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_DEPRECATE")
ELSE ()
    IF (CMAKE_C_COMPILER_ID MATCHES "[cC][lL][aA][nN][gG]")
        SET(CLANG true)
    ELSEIF (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        SET(GCC true)
    ENDIF ()

    STRING(APPEND CMAKE_C_FLAGS_DEBUG " -O0 -g3")
    STRING(APPEND CMAKE_C_FLAGS " -Wall")
    STRING(APPEND CMAKE_C_FLAGS " -Wno-missing-braces")
    OPTION(AMUL_SANITIZERS "Enable address/undefined sanitizers" ON)
    IF (AMUL_SANITIZERS)
        STRING(APPEND CMAKE_C_FLAGS_DEBUG " -fsanitize=address -fsanitize=undefined")
        #STRING(APPEND CMAKE_C_FLAGS_DEBUG " -fsanitize=memory")
        #STRING(APPEND CMAKE_C_FLAGS_DEBUG " -fsanitize=safe-stack")
        STRING(APPEND CMAKE_C_FLAGS_DEBUG " -fno-omit-frame-pointer")
        #STRING(APPEND CMAKE_C_FLAGS_DEBUG " -fsanitize-stats")
        IF (CLANG)
            STRING(APPEND CMAKE_C_FLAGS_DEBUG " -fprofile-instr-generate -fcoverage-mapping")
        ENDIF ()
    ENDIF ()
ENDIF ()

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

