SET(AMUL_LIB_SOURCES
		atom.cpp
        filesystem.cpp
		constants.cpp
        logging.cpp
		modules.cpp
		spinlock.cpp
		system.cpp
)

SET(AMUL_LIB_HEADERS
		atom.h
		atomtype.h
		buffer.h
		filesystem.h
		filesystem.inl.h
		modules.h
		msgports.h
		sourcefile.h
		spinlock.h
		system.h

        ../h/amul.alog.h
		../h/amul.cons.h
		../h/amul.list.h
)

SET(AMULCOM_SOURCES
        amulcom.cpp
		amulcom.cmdline.cpp
        amulcom.main.cpp
		amulcom.extras.cpp
		amulcom.strings.cpp
		tokenstream.cpp
)

SET(AMULCOM_HEADERS
        amulcom.h
		amulcom.strings.h
		atomtype.h
		char-to-atom.h
		token.h
		tokentype.h

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

SET(AMUL_MANAGER_NAME aman)
SET(AMUL_MANAGER_SOURCES
    aman.cpp
)
SET(AMUL_MANAGER_HEADERS
    system.h

    ${AMUL_HEADERS}
)


ADD_EXECUTABLE(
    ${AMUL_MANAGER_NAME}

    ${AMUL_MANAGER_SOURCES}
    ${AMUL_MANAGER_HEADERS}
)
TARGET_LINK_LIBRARIES(${AMUL_MANAGER_NAME}
    PUBLIC "${AMUL_LIB_NAME}")
