# Set required variables
# TODO Final configuration support


# TODO this is chosen by platform, instead it should probably be chosen
#      by compiler (WIN32 options are MSVC-specific)
IF(WIN32)
    ADD_COMPILE_DEFINITIONS(NFE_PLATFORM_WINDOWS)

    SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} /std:c++17")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob3 /Oi /Oy /MD")
    SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /Od /ZI /MDd")

    ADD_COMPILE_DEFINITIONS(NFE_USE_SSE)
    ADD_COMPILE_DEFINITIONS(NFE_USE_FMA)
    ADD_COMPILE_DEFINITIONS(NFE_USE_FP16C)
    ADD_COMPILE_DEFINITIONS(NFE_USE_AVX)
    ADD_COMPILE_DEFINITIONS(NFE_USE_AVX2)
    ADD_COMPILE_DEFINITIONS(TRACY_ENABLE TRACY_ON_DEMAND)
    ADD_COMPILE_DEFINITIONS(_HAS_EXCEPTIONS=0)
    ADD_COMPILE_DEFINITIONS(_CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)
    ADD_COMPILE_DEFINITIONS(WIN32 _WINDLL _WINDOWS)
    ADD_COMPILE_DEFINITIONS(_UNICODE)
    #ADD_COMPILE_DEFINITIONS(NFE_USE_AVX512)
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /arch:AVX2")

    # MSVC specific settings
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX") # warnings level 4, treat warnings as errors
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GF") # string pooling
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # build with multiple processes
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC") # use full paths
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gy") # func-level linking
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast") # fast float model
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:except-") # no float exceptions

    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GS-") # buffer security off

    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        ADD_COMPILE_DEFINITIONS(NFE_CONFIGURATION_DEBUG _DEBUG)
    ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
        ADD_COMPILE_DEFINITIONS(NFE_CONFIGURATION_RELEASE NDEBUG)
    ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

    # TODO Get rid of these deprecation warnings
    ADD_COMPILE_DEFINITIONS(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
ELSEIF(UNIX)
    ADD_COMPILE_DEFINITIONS(NFE_PLATFORM_LINUX)

    SET(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS} -DNFE_CONFIGURATION_RELEASE -O2 -std=c++17")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNFE_CONFIGURATION_DEBUG -D_DEBUG -O0 -g -std=c++17")

    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -msse4.1 -mpclmul -DNFE_USE_SSE")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx -mbmi2 -DNFE_USE_AVX")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx2 -DNFE_USE_AVX2")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mfma -DNFE_USE_FMA")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mf16c -DNFE_USE_FP16C")
    #SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx512f -mavx512bw -DNFE_USE_AVX512")

    # allow anonymous structs and unions
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fms-extensions")

    # Enable all warnings and make them errors
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wextra -Wno-sign-compare -Werror")

    # Warnings exceptions
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-multichar")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-switch")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unused-result")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unused-private-field")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-four-char-constants")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-gnu-anonymous-struct -Wno-nested-anon-types")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-format-security")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-self-assign-overloaded")

    # Warning exceptions for Release build
    # We make sure of avoiding these situations in Debug via asserts.
    # Asserts are compiled out on Release build, causing below excessive errors.
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-maybe-uninitialized")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-array-bounds")

    # Sanitizer build (clang or gcc >= 4.8 required)
    IF(DEFINED SANITIZE)
        MESSAGE(STATUS "Enabling sanitizer: ${SANITIZE}")
        SET(SANITIZE_FLAG          "-fsanitize=${SANITIZE}")
        SET(CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} ${SANITIZE_FLAG}")
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAG}")
    ENDIF(DEFINED SANITIZE)
ENDIF(WIN32)

ADD_SUBDIRECTORY("Engine")
ADD_SUBDIRECTORY("Tests")
#ADD_SUBDIRECTORY("Benchmarks")
ADD_SUBDIRECTORY("Apps")
