SET(AMUL_LIB_SOURCES
		amullib.cpp
		atom.cpp
		cmdline.cpp
		exceptions.cpp
		filemapping.cpp
        filesystem.cpp
		constants.cpp
        logging.cpp
		modules.cpp
		msgports.cpp
        sourcefile.cpp
		spinlock.cpp
        stringmanip.cpp
		system.cpp
)

SET(AMUL_LIB_HEADERS
		amullib.h
		atom.h
		atomtype.h
		buffer.h
		filemapping.h
		filesystem.h
		filesystem.inl.h
        logging.h
		modules.h
		msgports.h
		sourcefile.h
		spinlock.h
        stringmanip.h
        svparse.h
		system.h

		amul.cons.h
		amul.list.h
)

SET(AMULCOM_SOURCES
		amulcom.cmdline.cpp
        amulcom.main.cpp
		amulcom.extras.cpp
		amulcom.fileparser.cpp
		amulcom.strings.cpp

		amulcom.msgfile.cpp
		amulcom.titlefile.cpp

		filedefaults.cpp

		tokenstream.cpp
)

SET(AMULCOM_HEADERS
        amulcom.h
		amulcom.fileparser.h
		amulcom.fileprocessing.h
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

SET(AMUL_NAME amul)
SET(AMUL_SOURCES
	amul.actions.cpp
	amul.client.cpp
	amul.cmdline.cpp
	amul.console.cpp
	amul.demonactions.cpp
	amul.main.cpp
	amul.members.cpp
	amul.server.cpp
	amul.stringmanip.cpp
	amul.wholeoriginal.cpp
	client.io.cpp
	parser.context.cpp
	users.cpp
)
SET(AMUL_HEADERS
    amul.algorithms.h
	amul.demonactions.h
    amulinc.h
	amul.stringmanip.h
	client.io.h
	demon.h
	exceptions.h
	logging.h
	message.base.h
	message.common.h
    message.execdemon.h
	message.execfn.h
	modules.h
	msgports.h
	parser.context.h
    parser.expression.h
    parser.token.h
	platforms.h
	spinlock.h
	system.h
	users.h

    ${AMUL_HEADERS}
)

ADD_EXECUTABLE(
    ${AMUL_NAME}

    ${AMUL_SOURCES}
    ${AMUL_HEADERS}
)
TARGET_LINK_LIBRARIES(${AMUL_NAME}
    PUBLIC "${AMUL_LIB_NAME}"
    PRIVATE "${CMAKE_THREAD_LIBS_INIT}")
