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
IF(CMAKE_BUILD_TYPE STREQUAL "Release")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-array-bounds")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Release")

ADD_SUBDIRECTORY("Engine")
ADD_SUBDIRECTORY("Tests")
#ADD_SUBDIRECTORY("Benchmarks")
ADD_SUBDIRECTORY("Apps")
