SET(AMUL_LIB_SOURCES
	./amigastubs.cpp
	./constants.cpp
	./logging.cpp
	./os.cpp
	./variables.cpp
)

SET(AMUL_LIB_HEADERS
	h/amigastubs.h
	h/amul.acts.h
	h/amul.cons.h
	h/amul.defs.h
	h/amul.incs.h
	h/amul.logging.h
	h/amul.msgs.h
	h/amul.resources.h
	h/amul.stct.h
	h/amul.vars.h
	h/os.h
)

ADD_LIBRARY(
	${AMUL_LIB_NAME}
	STATIC
	${AMUL_LIB_SOURCES}
)