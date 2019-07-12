SET(TESTS_SOURCES
    testmain.c

    hashmap_test.c
)

SET(TESTS_HEADERS
    ../src/hashmap.h

    ${AMUL_LIB_SOURCES}

    ${AMUL_HEADERS}
)

ADD_EXECUTABLE(
    tests

    ${TESTS_SOURCES}
    ${TESTS_HEADERS}
)

TARGET_LINK_LIBRARIES(tests "${AMUL_LIB_NAME}")
