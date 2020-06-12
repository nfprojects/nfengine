CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(nfEngineDeps)

# in case we get called as root CMakeLists.txt, ex as a part of MSVC build process
IF(NOT DEFINED NFEDEPS_ROOT_DIRECTORY)
    SET(NFEDEPS_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(NOT DEFINED NFEDEPS_ROOT_DIRECTORY)

IF(NOT DEFINED NFE_BUILD_PLATFORM)
    IF(WIN32)
        SET(NFE_BUILD_PLATFORM ${CMAKE_GENERATOR_PLATFORM})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND uname -m
                        OUTPUT_VARIABLE NFE_BUILD_PLATFORM OUTPUT_STRIP_TRAILING_WHITESPACE)
    ELSE(WIN32)
        MESSAGE(FATAL_ERROR "Unsupported build platform")
    ENDIF(WIN32)
ENDIF(NOT DEFINED NFE_BUILD_PLATFORM)

# -fPIC must be set only for Unix shared objects
IF(UNIX)
    ADD_DEFINITIONS("-fPIC")
ENDIF(UNIX)

# force using static runtime linking
IF(WIN32)
    IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        ADD_DEFINITIONS("/MDd")
    ELSE("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        ADD_DEFINITIONS("/MD")
    ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
ENDIF(WIN32)


# Add dependencies with respective settings

# zlib
SET(SKIP_INSTALL_ALL ON)
ADD_SUBDIRECTORY(zlib)

# libpng
SET(ZLIB_LIBRARY zlibstatic)
SET(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib/ ${CMAKE_CURRENT_BINARY_DIR}/zlib)
SET(PNG_SHARED OFF CACHE BOOL "Build shared lib")
SET(PNG_TESTS OFF CACHE BOOL "Build libpng tests")
SET(PNG_BUILD_ZLIB ON CACHE BOOL "Custom zlib Location, else find_package is used")
ADD_SUBDIRECTORY(libpng)

# glslang
SET(ENABLE_SPVREMAPPER OFF CACHE BOOL "Enables building of SPVRemapper")
SET(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap")
SET(ENABLE_CTEST OFF CACHE BOOL "Enables testing")
SET(BUILD_EXTERNAL OFF CACHE BOOL "Build external dependencies in /External")
ADD_SUBDIRECTORY(glslang)

# googletest
SET(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
SET(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)")
ADD_SUBDIRECTORY(googletest)

# remainders which don't need additional options
ADD_SUBDIRECTORY(freetype2)
ADD_SUBDIRECTORY(libsquish)
ADD_SUBDIRECTORY(nfenginedeps)


# Determine one specified dir for outputs
IF(NOT NFEDEPS_LIB_DIRECTORY)
    SET(NFEDEPS_LIB_DIRECTORY ${NFEDEPS_ROOT_DIRECTORY}/Bin/${NFE_BUILD_PLATFORM}/${CMAKE_BUILD_TYPE}/ CACHE INTERNAL "nfEngine Deps output dir")
ENDIF(NOT NFEDEPS_LIB_DIRECTORY)


# Copy outputs via a custom target
ADD_CUSTOM_TARGET(NFEDepsPostBuild ALL DEPENDS glslang jpeg squish png_static zlibstatic
    COMMENT "Commands called post-build NFE dependencies"
)

IF(WIN32)
    ADD_CUSTOM_COMMAND(TARGET NFEDepsPostBuild POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/post_build.bat
    )
ELSEIF(UNIX)
    ADD_CUSTOM_COMMAND(TARGET NFEDepsPostBuild POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/post_build.sh
    )
ENDIF()

ADD_CUSTOM_COMMAND(TARGET NFEDepsPostBuild POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:squish> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:jpeg> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:png_static> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:zlibstatic> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:glslang> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:HLSL> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OSDependent> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OGLCompiler> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SPIRV> ${NFEDEPS_LIB_DIRECTORY}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest> ${NFEDEPS_LIB_DIRECTORY}
                   VERBATIM
)

FILE(MAKE_DIRECTORY ${NFEDEPS_LIB_DIRECTORY})
