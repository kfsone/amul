# On Unix, create a target for building the CTAGS file.
IF ( UNIX )
	FIND_PROGRAM(CTAGS ctags)
	ADD_CUSTOM_TARGET(ctags COMMAND ${CTAGS} -R ${CMAKE_SOURCE_DIR}/*/*.cpp ${CMAKE_SOURCE_DIR}/*/*.h)
ENDIF ( UNIX )
