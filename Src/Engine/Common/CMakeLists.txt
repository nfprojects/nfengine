# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for Common

MESSAGE("Generating Makefile for Common")

SET(NFE_COMMON_DIRECTORY ${NFE_ENGINE_DIRECTORY}/Common)

FILE(GLOB_RECURSE NFCOMMON_SOURCES *.cpp)
FILE(GLOB_RECURSE NFCOMMON_HEADERS *.hpp)

# Windows-specific files
IF(NOT WIN32)
    LIST(REMOVE_ITEM NFCOMMON_SOURCES "Main.cpp")
    LIST(FILTER NFCOMMON_SOURCES EXCLUDE REGEX "(.*)/Windows/(.*).cpp")
    LIST(FILTER NFCOMMON_HEADERS EXCLUDE REGEX "(.*)/Windows/(.*).hpp")
ENDIF(NOT WIN32)

# Search for dependencies
PKG_CHECK_MODULES(NFE_COMMON_DEPS REQUIRED xcb xcb-image)

# setup directories
ADD_LIBRARY(Common SHARED ${NFCOMMON_SOURCES} ${NFCOMMON_HEADERS})

TARGET_INCLUDE_DIRECTORIES(Common
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/nfenginedeps
                           PRIVATE ${NFE_COMMON_DIRECTORY})

TARGET_LINK_DIRECTORIES(Common
                        PRIVATE ${NFEDEPS_LIB_DIRECTORY})

ADD_DEPENDENCIES(Common NFEDepsPostBuild)
TARGET_PRECOMPILE_HEADERS(Common PRIVATE PCH.hpp)
TARGET_LINK_LIBRARIES(Common squish jpeg png_static dl ${NFE_COMMON_DEPS_LIBRARIES})

ADD_CUSTOM_COMMAND(TARGET Common POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Common> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
