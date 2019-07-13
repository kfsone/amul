SET(AMUL_LIB_SOURCES
        filesystem.c
		constants.c
        hashmap.c
        logging.c
		modules.c
		system.c
)

SET(AMUL_LIB_HEADERS
        hashmap.h
		modules.h
		system.h

        ../h/amul.alog.h
		../h/amul.cons.h
        ../h/amul.file.h
        ../h/amul.hash.h
		../h/amul.list.h
)

SET(AMULCOM_SOURCES
        amulcom.c
        amulcom.main.c
		amulcom.extras.c
		amulcom.runtime.c
		amulcom.strings.c
)

SET(AMULCOM_HEADERS
        amulcom.h
		amulcom.runtime.h
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
