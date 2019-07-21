# Check for various include files.
INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILES(memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILES(sys/wait.h HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILES(sys/resource.h HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILES(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES(sys/param.h HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILES(mman.h HAVE_MMAN_H)
CHECK_INCLUDE_FILES(sys/mman.h HAVE_SYS_MMAN_H)

# Check whether various functions are defined in various headers.
INCLUDE(CheckSymbolExists)
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
ELSE ( )
 ADD_DEFINITIONS("/D_CRT_SECURE_NO_WARNINGS")
ENDIF ( )
