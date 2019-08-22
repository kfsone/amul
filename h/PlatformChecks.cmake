# Perform cross-platform tests and configure a config.h file

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckTypeSize)

SET(CONFIG_H "amulconfig.h")

ADD_COMPILE_DEFINITIONS("HAVE_CONFIG_H=${CONFIG_H}")

# Compiler identification
IF (MSVC)
    ADD_COMPILE_DEFINITIONS("_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_DEPRECATE")
	STRING(APPEND CMAKE_CXX_FLAGS "/W4")
ELSE ()
    IF (CMAKE_C_COMPILER_ID MATCHES "[cC][lL][aA][nN][gG]")
        SET(CLANG true)
    ELSEIF (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        SET(GCC true)
    ENDIF ()

	STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0 -g3")
	STRING(APPEND CMAKE_CXX_FLAGS " -Wall -Wshadow -Wwrite-strings")
ENDIF()

# Check for various include files.
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(sys/wait.h HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILES(mman.h HAVE_MMAN_H)

# Check whether various functions are defined in various headers.
CHECK_INCLUDE_FILES(sys/mman.h HAVE_SYS_MMAN_H)
IF ( NOT WIN32 )
 IF ( HAVE_MMAN_H AND NOT HAVE_MMAP )
  CHECK_SYMBOL_EXISTS(mmap "mman.h" HAVE_MMAP)
 ENDIF ( )
 IF ( HAVE_SYS_MMAN_H AND NOT HAVE_MMAP )
  CHECK_SYMBOL_EXISTS(mmap "sys/mman.h" HAVE_MMAP)
 ENDIF ( )
 IF ( NOT HAVE_MMAP )
  MESSAGE(SEND_ERROR "Don't have a way to memory-map files")
 ENDIF ( )
ENDIF ( )

CHECK_TYPE_SIZE(off_t OFF_T)
CHECK_TYPE_SIZE(error_t ERROR_T)
CHECK_TYPE_SIZE(ssize_t SSIZE_T)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/h/${CONFIG_H}.in
	${CMAKE_BINARY_DIR}/${CONFIG_H}
)
