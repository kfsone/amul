OPTION(AMUL_INCLUDE_TESTS "Build and run unit tests" ON)

include(GoogleTest)

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
	#gtest_discover_tests(${TESTNAME})
	add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
	set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
ENDMACRO()

add_gtest(AtomTest ${AMUL_LIB_NAME} test_atom.cpp ../src/atom.h ../src/atomtype.h ../src/buffer.h)
add_gtest(BufferTest ${AMUL_LIB_NAME} test_buffer.cpp ../src/buffer.h)
add_gtest(FileMappingTest ${AMUL_LIB_NAME} test_filemapping.cpp ../src/filemapping.h ../src/filesystem.h)
add_gtest(FilesystemTest ${AMUL_LIB_NAME} test_filesystem.cpp ../src/filesystem.h)
add_gtest(ModuleTest ${AMUL_LIB_NAME} test_module.cpp ../src/modules.h)
add_gtest(MsgPortTest ${AMUL_LIB_NAME} test_msgport.cpp ../src/system.h ../src/msgports.h)
add_gtest(SpinLockTest ${AMUL_LIB_NAME} test_spinlock.cpp ../src/spinlock.h ../src/spinlock.h)

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

