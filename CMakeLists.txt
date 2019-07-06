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

add_subdirectory(lib)
add_subdirectory(compiler)
#add_subdirectory(engine)
