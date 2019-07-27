OPTION(AMUL_INCLUDE_TESTS "Build and run unit tests" ON)

IF (AMUL_INCLUDE_TESTS)

	MESSAGE(STATUS "Testing enabled")

	ENABLE_TESTING()

	INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/h")
	INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src")

	SET(gtest_force_shared_crt ON CACHE BOOL "Force gtest to use shared crt" FORCE)
	ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/tests/googletest" "tests/googletest")

	MARK_AS_ADVANCED(
		BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
		gmock_build_tests gtest_build_samples gtest_build_tests
		gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
	)

	MACRO(add_gtest TESTNAME LINKLIB)
		add_executable(${TESTNAME} ${ARGN})
		target_link_libraries(${TESTNAME} gtest gmock gtest_main ${AMUL_LIB_NAME} ${LINKLIB})
		add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
		set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
	ENDMACRO()

ELSE ()

	MACRO(add_gtest TESTNAME LINKLIB)
	ENDMACRO()

	ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/tests/googletest" "tests/googletest" EXCLUDE_FROM_ALL)

ENDIF()


add_gtest(BufferTest ${AMUL_LIB_NAME} test_buffer.cpp ../src/buffer.h)
add_gtest(FilesystemTest ${AMUL_LIB_NAME} test_filesystem.cpp ../src/filesystem.h)
add_gtest(HashmapTest ${AMUL_LIB_NAME} test_hashmap.cpp ../src/hashmap.h)
add_gtest(ModuleTest ${AMUL_LIB_NAME} test_module.cpp ../src/modules.h)

#SET(TESTS_SOURCES
#    testmain.cpp
#
#    buffer_tests.cpp
#    filesystem_tests.cpp
#    hashmap_tests.cpp
#	module_tests.cpp
#)
#
#SET(TESTS_HEADERS
#    ../src/buffer.h
#    ../src/filesystem.h
#    ../src/hashmap.h
#    ../src/modules.h
#    ../src/sourcefile.h
#
#    ${AMUL_LIB_SOURCES}
#
#    ${AMUL_HEADERS}
#)
#
#ADD_EXECUTABLE(
#    libtests
#
#    ${TESTS_SOURCES}
#    ${TESTS_HEADERS}
#)
#
#TARGET_LINK_LIBRARIES(libtests "${AMUL_LIB_NAME}")
#
#ADD_TEST(NAME unittests COMMAND libtests)

