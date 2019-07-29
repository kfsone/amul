CMAKE_MINIMUM_REQUIRED (VERSION 3.10)
PROJECT( SMUGL )

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE(include/PlatformChecks.cmake)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/config.h.in ${SMUGL_BINARY_DIR}/config.h)

# On Unix, create a target for building the CTAGS file.
IF ( UNIX )
	FIND_PROGRAM(CTAGS ctags)
	ADD_CUSTOM_TARGET(ctags COMMAND ${CTAGS} -R ${CMAKE_SOURCE_DIR}/*/*.cpp ${CMAKE_SOURCE_DIR}/*/*.hpp)
ENDIF ( UNIX )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${SMUGL_BINARY_DIR})

######################################################################
# Definition of what build this is.

add_custom_target(
	revision.h
	COMMAND perl ${CMAKE_SOURCE_DIR}/make-revision.py -i project.rev -p Smugl -o ${SMUGL_BINARY_DIR}/revision.h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS ${CMAKE_SOURCE_DIR}/make-revision.py
)

######################################################################
# Various pieces of common code in the library.

add_library(
	smuglcommon
		lib/actions.cpp
		lib/classes.cpp
		lib/conditions.cpp
		lib/data.cpp
		lib/mappedfile.cpp
		lib/misc.cpp
		lib/syslog.cpp
		lib/vocab.cpp
)

######################################################################
# SMUGL's compiler.

add_executable(
	smuglcom
		smuglcom/basicobjs.cpp
		smuglcom/cleanup.cpp
		smuglcom/errors.cpp
		smuglcom/filebits.cpp
		smuglcom/langproc.cpp
		smuglcom/misc.cpp
		smuglcom/mobproc.cpp
		smuglcom/msgmgt.cpp
		smuglcom/obdsproc.cpp
		smuglcom/objproc.cpp
		smuglcom/roomproc.cpp
		smuglcom/smsgproc.cpp
		smuglcom/smuglcom.cpp
		smuglcom/synsproc.cpp
		smuglcom/titleproc.cpp
		smuglcom/travfunc.cpp
		smuglcom/travproc.cpp
		smuglcom/umsgproc.cpp
		smuglcom/vocab.cpp
		smuglcom/version.cpp
)
add_dependencies(smuglcom revision.h)
target_link_libraries(smuglcom smuglcommon)

######################################################################
# Compiled-data validation tool.

add_executable(
	verify
		smuglcom/verify.cpp
		smuglcom/version.cpp
)
add_dependencies(verify revision.h)
target_link_libraries(verify smuglcommon)

######################################################################
# The SMUGL runtime.

add_executable(
	smugl
		smugl/aliases.cpp
		smugl/basicobjs.cpp
		smugl/client.cpp
		smugl/io.cpp
		smugl/ipc.cpp
		smugl/lang.cpp
		smugl/langtable.cpp
		smugl/loaders.cpp
		smugl/login.cpp
		smugl/manager.cpp
		smugl/misc.cpp
		smugl/mobiles.cpp
		smugl/objects.cpp
		smugl/parser.cpp
		smugl/players.cpp
		smugl/ranks.cpp
		smugl/rooms.cpp
		smugl/smugl.cpp
		smugl/textproc.cpp
		smugl/travel.cpp
		smugl/version.cpp
)
add_dependencies(smugl revision.h)
target_link_libraries(smugl smuglcommon)

######################################################################
# Testbed.

add_executable(
	testbed
		lib/testbed.cpp
		lib/mappedfile.cpp
)
target_link_libraries(testbed smuglcommon)

