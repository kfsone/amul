SET(AMUL_COMPILER_SOURCES
	amulcom.cpp

	constants.cpp
	context.cpp
	extras.cpp
	file_handling.cpp
	travel_functions.cpp

	parsers/language.cpp
	parsers/mobile.cpp
	parsers/object.cpp
	parsers/object_description.cpp
	parsers/rank.cpp
	parsers/room.cpp
	parsers/synonyms.cpp
	parsers/system_message.cpp
	parsers/title.cpp
	parsers/travel.cpp
	parsers/user_message.cpp
)

SET(AMUL_COMPILER_HEADERS
	amulcom.h
	amulcom.includes.h
	constants.h
	context.h
	extras.h
	file_handling.h
	travel_functions.h
	variables.h

	parsers/parsers.h

	${AMUL_LIB_HEADERS}
)

include_directories(".")

add_library(
	"${AMUL_COMPILER_NAME}-lib"

	${AMUL_COMPILER_SOURCES}
)
target_compile_definitions("${AMUL_COMPILER_NAME}-lib" PRIVATE COMPILER=1)

add_executable(
	${AMUL_COMPILER_NAME}

	main.cpp
	${AMUL_COMPILER_HEADERS}
)
target_link_libraries(${AMUL_COMPILER_NAME} ${AMUL_LIB_NAME} ${AMUL_COMPILER_NAME}-lib)
target_compile_definitions("${AMUL_COMPILER_NAME}" PRIVATE COMPILER=1)
