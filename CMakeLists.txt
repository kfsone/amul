# CMakeList.txt : CMake project for AMUL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

SET(AMUL_LIB_NAME "amullib")
SET(AMUL_COMPILER_NAME "amulcom")
SET(AMUL_ENGINE_NAME "amul")

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")

IF (MSVC)
	add_compile_definitions("_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_DEPRECATE")
ENDIF ()

OPTION(PACKAGE_TESTS "Build tests" ON)
IF (PACKAGE_TESTS)
	enable_testing()
	SET(gtest_force_shared_crt ON CACHE BOOL "Force gtest to use shared crt" FORCE)
	add_subdirectory("${PROJECT_SOURCE_DIR}/tests/googletest" "tests/googletest")

	mark_as_advanced(
		BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
		gmock_build_tests gtest_build_samples gtest_build_tests
		gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
	)

	macro(add_gtest TESTNAME LINKLIB)
		add_executable(${TESTNAME} ${ARGN})
		target_link_libraries(${TESTNAME} gtest gmock gtest_main ${AMUL_LIB_NAME} ${LINKLIB})
		add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
		set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
	endmacro()
	
	add_subdirectory(tests)
ENDIF()

add_subdirectory(lib)
add_subdirectory(compiler)
#add_subdirectory(engine)
