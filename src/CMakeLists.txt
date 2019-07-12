SET(AMUL_LIB_SOURCES
        filesystem.c
        hashmap.c
        logging.c
)

SET(AMUL_LIB_HEADERS
        hashmap.h

        ../h/amul.alog.h
        ../h/amul.file.h
        ../h/amul.hash.h
)

SET(AMULCOM_SOURCES
        amulcom.main.c
        amulcom.c
        extras.c
)

SET(AMULCOM_HEADERS
        amulcom.h

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
