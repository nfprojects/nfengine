CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(nfEngineDeps)

# in case we get called as root CMakeLists.txt, ex as a part of MSVC build process
IF(NOT DEFINED NFEDEPS_ROOT_DIRECTORY)
    SET(NFEDEPS_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(NOT DEFINED NFEDEPS_ROOT_DIRECTORY)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

IF(NOT DEFINED NFE_BUILD_PLATFORM)
    IF(WIN32)
        SET(NFE_BUILD_PLATFORM ${CMAKE_GENERATOR_PLATFORM})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND uname -m
            OUTPUT_VARIABLE NFE_BUILD_PLATFORM OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    ELSE(WIN32)
        MESSAGE(FATAL_ERROR "Unsupported build platform")
    ENDIF(WIN32)
ENDIF(NOT DEFINED NFE_BUILD_PLATFORM)

# -fPIC must be set only for Unix shared objects
IF(UNIX)
    ADD_DEFINITIONS("-fPIC")
ENDIF(UNIX)


# Disable testing via CTest - might shave off some targets
SET(ENABLE_CTEST OFF CACHE BOOL "Enables testing")
SET(BUILD_TESTING OFF CACHE BOOL "Build testing")


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
SET(ENABLE_OPT ON CACHE BOOL "Enables spirv-opt capability if present")
SET(BUILD_EXTERNAL ON CACHE BOOL "Build external dependencies in /External")
SET(SKIP_GLSLANG_INSTALL ON CACHE BOOL "Skip installation")

IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/glslang/External/spirv-tools)
    MESSAGE(FATAL_ERROR "SPIRV-Tools are not downloaded for Glslang build. Please enter Deps/glslang and run update_glslang_sources.py")
ENDIF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/glslang/External/spirv-tools)

ADD_SUBDIRECTORY(glslang)

# googletest
SET(build_gmock OFF CACHE BOOL "Builds the googlemock subproject")
SET(install_gtest OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)")
SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
ADD_SUBDIRECTORY(googletest)

# tinyexr
SET(TINYEXR_BUILD_SAMPLE OFF CACHE BOOL "Build a sample")
ADD_SUBDIRECTORY(tinyexr)

# remainders which don't need additional options
ADD_SUBDIRECTORY(freetype2)
ADD_SUBDIRECTORY(libsquish)
ADD_SUBDIRECTORY(tinyobjloader)
ADD_SUBDIRECTORY(cxxopts)
ADD_SUBDIRECTORY(nfenginedeps)


## FAKE TARGETS for parts which don't have easy access to their CMakeLists targets ##

# D3D12MemAllocator has to be built separately due to no CMake and warnings produced
IF(WIN32)
    SET(NFEDEPS_D3D12MEMALLOC_SRC_DIR D3D12MemoryAllocator/src)
    SET(NFEDEPS_D3D12MEMALLOC_SOURCES
        ${NFEDEPS_D3D12MEMALLOC_SRC_DIR}/D3D12MemAlloc.cpp
        ${NFEDEPS_D3D12MEMALLOC_SRC_DIR}/Common.cpp
    )
    ADD_LIBRARY(D3D12MemoryAllocator ${NFEDEPS_D3D12MEMALLOC_SOURCES})
ENDIF(WIN32)


# Determine one specified dir for outputs
IF(NOT NFEDEPS_LIB_DIRECTORY)
    SET(NFEDEPS_LIB_DIRECTORY ${NFEDEPS_ROOT_DIRECTORY}/Bin/${NFE_BUILD_PLATFORM}/${CMAKE_BUILD_TYPE}/ CACHE INTERNAL "nfEngine Deps output dir")
ENDIF(NOT NFEDEPS_LIB_DIRECTORY)


# Copy outputs via a custom target
ADD_CUSTOM_TARGET(NFEDepsPostBuild ALL
    DEPENDS glslang glslang-default-resource-limits jpeg squish png_static zlibstatic gtest freetype
            SPIRV-Tools miniz tinyexr tinyobjloader cxxopts
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
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:glslang-default-resource-limits> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:HLSL> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OSDependent> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MachineIndependent> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:GenericCodeGen> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OGLCompiler> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SPIRV> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SPIRV-Tools> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SPIRV-Tools-opt> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:freetype> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:miniz> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tinyexr> ${NFEDEPS_LIB_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tinyobjloader> ${NFEDEPS_LIB_DIRECTORY}
    VERBATIM
)

IF(WIN32)
    ADD_CUSTOM_COMMAND(TARGET NFEDepsPostBuild POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:D3D12MemoryAllocator> ${NFEDEPS_LIB_DIRECTORY}
        VERBATIM
    )
ENDIF(WIN32)


# Group targets under "Deps" filter - helps if this is built as part of main Win Engine solution
INCLUDE(NFEGetAllTargets)
NFE_GET_ALL_TARGETS(DEPS_TARGETS)

FOREACH(DEP ${DEPS_TARGETS})
    GET_TARGET_PROPERTY(DEP_TYPE ${DEP} TYPE)
    # Interface libraries don't have FOLDER property, thus we have to work around them
    IF(NOT "${DEP_TYPE}" STREQUAL "INTERFACE_LIBRARY")
        GET_PROPERTY(DEP_FOLDER TARGET ${DEP} PROPERTY FOLDER)
        SET_PROPERTY(TARGET ${DEP} PROPERTY FOLDER "Deps/${DEP_FOLDER}")
    ENDIF(NOT "${DEP_TYPE}" STREQUAL "INTERFACE_LIBRARY")
ENDFOREACH()


FILE(MAKE_DIRECTORY ${NFEDEPS_LIB_DIRECTORY})
