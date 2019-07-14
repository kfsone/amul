SET(AMUL_LIB_SOURCES
		buffer.c
        filesystem.c
		constants.c
        hashmap.c
        logging.c
		modules.c
		system.c
		tokenizer.c
)

SET(AMUL_LIB_HEADERS
		buffer.h
		filesystem.h
        hashmap.h
		modules.h
		sourcefile.h
		system.h
		tokenizer.h

        ../h/amul.alog.h
		../h/amul.cons.h
        ../h/amul.hash.h
		../h/amul.list.h
)

SET(AMULCOM_SOURCES
        amulcom.c
		amulcom.cmdline.c
        amulcom.main.c
		amulcom.extras.c
		amulcom.strings.c
)

SET(AMULCOM_HEADERS
        amulcom.h
		amulcom.strings.h

        ${AMUL_HEADERS}
)

ADD_LIBRARY(
    ${AMUL_LIB_NAME}

    ${AMUL_LIB_SOURCES}
)

ADD_EXECUTABLE(
    ${AMUL_COMPILER_NAME}

    ${AMULCOM_SOURCES}
    ${AMULCOM_HEADERS}
)
TARGET_LINK_LIBRARIES(${AMUL_COMPILER_NAME}
    PUBLIC "${AMUL_LIB_NAME}")
TARGET_COMPILE_DEFINITIONS(${AMUL_COMPILER_NAME} PRIVATE COMPILER=1)
