CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(AMUL C)

SET(AMUL_LIB_NAME "amullib")
SET(AMUL_COMPILER_NAME "amulcom")
SET(AMUL_ENGINE_NAME "amul")
FILE(GLOB AMUL_HEADERS ${PROJECT_SOURCE_DIR}/h/amul.*.h)

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED ON)

SET(CMAKE_C_FLAGS "-std=c11 -Og -g3 -DDEBUG -D_DEBUG -DDEBUG_")
STRING(APPEND CMAKE_C_FLAGS " -Wall")
STRING(APPEND CMAKE_C_FLAGS " -I${PROJECT_SOURCE_DIR}")
STRING(APPEND CMAKE_C_FLAGS " -Wno-format -Wno-missing-braces -Wno-int-conversion")
STRING(APPEND CMAKE_C_FLAGS " -Wno-incompatible-pointer-types-discards-qualifiers -Wno-tautological-constant-out-of-range-compare")
STRING(APPEND CMAKE_C_FLAGS " -Wno-discarded-qualifiers -Wno-unused-result")
OPTION(AMUL_SANITIZERS "Enable address/undefined sanitizers" ON)
IF (AMUL_SANITIZERS)
    STRING(APPEND CMAKE_C_FLAGS " -fsanitize=address,undefined")
    STRING(APPEND CMAKE_C_FLAGS " -fno-omit-frame-pointer")
ENDIF ()

IF (MSVC)
	add_compile_definitions("_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_DEPRECATE")
ENDIF ()

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

