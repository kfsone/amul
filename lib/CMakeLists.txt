SET(AMUL_LIB_SOURCES
	amigastubs.cpp
	logging.cpp
	os.cpp
)

SET(AMUL_LIB_HEADERS
	h/amul.acts.h
	h/amigastubs.h
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
	${AMUL_LIB_SOURCES}
)