# On Unix, create a target for building the CTAGS file.
IF ( UNIX )
	FIND_PROGRAM(CTAGS ctags)
	ADD_CUSTOM_TARGET(tags COMMAND ${CTAGS} -R ${CMAKE_SOURCE_DIR})
	ADD_CUSTOM_TARGET(ctags DEPENDS tags)
	MESSAGE(STATUS "Enabled ctags")
ENDIF ( UNIX )
