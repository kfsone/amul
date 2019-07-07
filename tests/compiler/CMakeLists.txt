include_directories("${PROJECT_SOURCE_DIR}/h")
include_directories("${PROJECT_SOURCE_DIR}/compiler")

add_gtest(ExtrasTest ${AMUL_COMPILER_NAME}-lib test_extras.cpp)
