# Perform cross-platform tests and configure a config.h file

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckTypeSize)

FIND_PACKAGE(Threads REQUIRED)

# Compiler identification
IF (MSVC)
    ADD_COMPILE_DEFINITIONS("_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_DEPRECATE")
	STRING(APPEND CMAKE_CXX_FLAGS " /wd4244 /wd4245 /wd4267 /w34138 /w34100")
ELSE ()
    IF (CMAKE_C_COMPILER_ID MATCHES "[cC][lL][aA][nN][gG]")
        SET(CLANG true)
    ELSEIF (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        SET(GCC true)
    ENDIF ()

	STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0 -g3")
	STRING(APPEND CMAKE_CXX_FLAGS " -Wall -Wshadow -Wwrite-strings -Wno-unused-variable -Wno-unused-label -Wno-int-to-pointer-cast")
ENDIF()

# Check for various include files.
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(sys/wait.h HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILE(mman.h HAVE_MMAN_H)

# Check whether various functions are defined in various headers.
CHECK_INCLUDE_FILE(sys/mman.h HAVE_SYS_MMAN_H)
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

CHECK_TYPE_SIZE(off_t OFF_T LANGUAGE CXX)
CHECK_TYPE_SIZE(error_t ERROR_T LANGUAGE CXX)
CHECK_TYPE_SIZE(ssize_t SSIZE_T LANGUAGE CXX)

CONFIGURE_FILE(${CONFIG_H_IN} ${CONFIG_H})