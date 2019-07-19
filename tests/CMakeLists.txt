SET(TESTS_SOURCES
    testmain.c

    buffer_tests.c
    filesystem_tests.c
    hashmap_tests.c
	module_tests.c
)

SET(TESTS_HEADERS
    ../src/hashmap.h

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

