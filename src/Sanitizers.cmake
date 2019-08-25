# Options for enabling sanitizers like address sanitizer

IF (NOT MSVC)
	SET(SANITIZERS OFF)

	# Available sanitizer options
	OPTION(AMUL_ASAN "Enable address sanitizer" ON)
	OPTION(AMUL_UBSAN "Enable undefined behavior sanitizer" ON)
	IF (CLANG)
		OPTION(AMUL_MSAN "Enable memory sanitizer" OFF)
		OPTION(AMUL_DFSAN "Enable dataflow sanitizer" OFF)
		OPTION(AMUL_COVERAGE "Enable code coverage" OFF)
	ENDIF ()

	# Validate any conflicts/check for any dependencies
	IF (AMUL_UBSAN AND AMUL_MSAN)
		MESSAGE(FATAL_ERROR "UBSAN and MSAN are mutually exclusive")
	ENDIF ()

	# Configure requested sanitizers
	IF (AMUL_ASAN)
		MESSAGE(STATUS "Address Sanitizer is ON")
        STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address")
		SET(SANITIZERS ON)
	ENDIF ()
	IF (AMUL_UBSAN)
		MESSAGE(STATUS "Undefined Behavior Sanitizer is ON")
		STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=undefined -fsanitize-recover=all")
		IF (CLANG)
			STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=undefined,nullability")
		ENDIF ()
		SET(SANITIZERS ON)
	ENDIF ()
	IF (AMUL_MSAN)
		MESSAGE(STATUS "Memory Sanitizer is ON")
		STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=memory ")
		IF (CLANG)
			STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize-memory-track-origins=2 -fsanitize-memory-use-after-dtor")
		ENDIF ()
		SET(SANITIZERS ON)
	ENDIF ()
	IF (CLANG AND AMUL_DFSAN)
		MESSAGE(STATUS "Undefined Behavior Sanitizer is ON")
		STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=dataflow")
		SET(SANITIZERS ON)
	ENDIF ()

	IF (SANITIZERS)
		STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fno-omit-frame-pointer")
		STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fno-optimize-sibling-calls")
	ENDIF ()

	# Non-sanitizers
	IF (CLANG AND AMUL_COVERAGE)
		STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fprofile-instr-generate -fcoverage-mapping")
	ENDIF ()
ENDIF ()
