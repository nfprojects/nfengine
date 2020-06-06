# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  Main CMake file for nfEngine

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(nfEngine)

MESSAGE("Generating Makefile for nfEngine project")

INCLUDE(FindPkgConfig)

# Pre-define Release build
IF(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")

# Pre-define 64-bit build platform
IF(NOT DEFINED BUILD_PLATFORM)
    EXECUTE_PROCESS(COMMAND uname -m
                    OUTPUT_VARIABLE BUILD_PLATFORM OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF(NOT DEFINED BUILD_PLATFORM)

# Set required variables
SET(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS} -O2 -std=c++17")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -O0 -g -std=c++17")

# Sanitizer build (clang or gcc >= 4.8 required)
IF(DEFINED SANITIZE)
    MESSAGE(STATUS "Enabling sanitizer: ${SANITIZE}")
    SET(SANITIZE_FLAG          "-fsanitize=${SANITIZE}")
    SET(CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} ${SANITIZE_FLAG}")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAG}")
ENDIF(DEFINED SANITIZE)

# If any macros specific to project are needed, prefix them with NFE_
SET(NFE_ROOT_DIRECTORY ${CMAKE_SOURCE_DIR})
SET(NFE_OUTPUT_DIRECTORY ${NFE_ROOT_DIRECTORY}/Bin/${BUILD_PLATFORM}/${CMAKE_BUILD_TYPE})
SET(NFEDEPS_LIB_DIRECTORY ${NFE_ROOT_DIRECTORY}/nfEngineDeps/Bin/${BUILD_PLATFORM})
SET(NFE_SRC_DIRECTORY ${NFE_ROOT_DIRECTORY}/Src)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -msse4.1 -mpclmul -DNFE_USE_SSE")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx -mbmi2 -DNFE_USE_AVX")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx2 -DNFE_USE_AVX2")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mfma -DNFE_USE_FMA")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mf16c -DNFE_USE_FP16C")

# allow anonymous structs and unions
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fms-extensions")

# disable C++ exceptions
# TODO some external libs require this, fix it
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fno-exceptions")

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

ADD_DEFINITIONS(-DNFE_ROOT_DIRECTORY="${NFE_ROOT_DIRECTORY}/")
ADD_DEFINITIONS(-DNFE_COMPILER="${CMAKE_CXX_COMPILER_ID}")
ADD_DEFINITIONS(-DNFE_OS="${CMAKE_SYSTEM}")

# Building outputs
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${NFE_OUTPUT_DIRECTORY})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${NFE_OUTPUT_DIRECTORY})

MESSAGE("Build type is ${CMAKE_BUILD_TYPE}")
MESSAGE("Output path is ${NFE_OUTPUT_DIRECTORY}")
MESSAGE("Platform is ${BUILD_PLATFORM}")

# Add all projects
ADD_SUBDIRECTORY("Src")

FILE(MAKE_DIRECTORY ${NFE_OUTPUT_DIRECTORY})
