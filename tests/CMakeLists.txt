SET(TESTS_SOURCES
    testmain.cpp

    buffer_tests.cpp
    filesystem_tests.cpp
    hashmap_tests.cpp
	module_tests.cpp
)

SET(TESTS_HEADERS
    ../src/buffer.h
    ../src/filesystem.h
    ../src/hashmap.h
    ../src/modules.h
    ../src/sourcefile.h

    ${AMUL_LIB_SOURCES}

    ${AMUL_HEADERS}
)

ADD_EXECUTABLE(
    libtests

    ${TESTS_SOURCES}
    ${TESTS_HEADERS}
)

TARGET_LINK_LIBRARIES(libtests "${AMUL_LIB_NAME}")

ADD_TEST(NAME unittests COMMAND libtests)

