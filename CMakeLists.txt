# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  Main CMake file for nfEngine

CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(nfEngine)

IF("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    MESSAGE(FATAL_ERROR
            "In-source build is not allowed. Please, make a build directory and "
            "run CMake there, ex:\n"
            "    $ mkdir build; cd build\n"
            "    $ cmake .."
            )
ENDIF("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")

MESSAGE(STATUS "Generating build files for nfEngine project")

IF(UNIX)
    INCLUDE(FindPkgConfig)
ENDIF(UNIX)

# Pre-define Release build
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Pre-define 64-bit build platform
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

# If any macros specific to project are needed, prefix them with NFE_
SET(NFE_ROOT_DIRECTORY ${CMAKE_SOURCE_DIR})
SET(NFE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})
SET(NFE_OUTPUT_DIRECTORY ${NFE_ROOT_DIRECTORY}/Bin/${NFE_BUILD_PLATFORM}/${CMAKE_BUILD_TYPE})
SET(NFEDEPS_ROOT_DIRECTORY ${NFE_ROOT_DIRECTORY}/Deps)
SET(NFEDEPS_LIB_DIRECTORY ${NFEDEPS_ROOT_DIRECTORY}/Bin/${NFE_BUILD_PLATFORM}/${CMAKE_BUILD_TYPE})
SET(NFE_SRC_DIRECTORY ${NFE_ROOT_DIRECTORY}/Src)

MESSAGE(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Project directory is ${NFE_ROOT_DIRECTORY}")
MESSAGE(STATUS "Build directory is ${NFE_BUILD_DIRECTORY}")
MESSAGE(STATUS "Output path is ${NFE_OUTPUT_DIRECTORY}")
MESSAGE(STATUS "Platform is ${NFE_BUILD_PLATFORM}")

# Add dependencies
ADD_SUBDIRECTORY("Deps")

IF(WIN32)
    # on Windows we build with Unicode support, thus string must be wide literal
    ADD_COMPILE_DEFINITIONS(NFE_ROOT_DIRECTORY=LR\"\(\"${NFE_ROOT_DIRECTORY}/\"\)\")
ELSEIF(UNIX)
    ADD_COMPILE_DEFINITIONS(NFE_ROOT_DIRECTORY="${NFE_ROOT_DIRECTORY}/")
ELSE(WIN32)
    MESSAGE(FATAL_ERROR "Target platform not supported")
ENDIF(WIN32)

ADD_COMPILE_DEFINITIONS(NFE_COMPILER="${CMAKE_CXX_COMPILER_ID}")
ADD_COMPILE_DEFINITIONS(NFE_OS="${CMAKE_SYSTEM}")

# Add all projects
ADD_SUBDIRECTORY("Src")

FILE(MAKE_DIRECTORY ${NFE_OUTPUT_DIRECTORY})

# Meta target to build engine and all required dependencies.
# Added because default ALL_BUILD will also produce lots of extra "bloat" builds from
# targets added by dependencies. We attach it to Demo Apps, which should pull other deps
# as needed.
ADD_CUSTOM_TARGET(Engine_All DEPENDS RendererDemo RaytracerDemo
    COMMENT "Build all Engine projects and dependencies"
)
