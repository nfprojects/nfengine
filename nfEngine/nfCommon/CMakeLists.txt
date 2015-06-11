# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for nfCommon

MESSAGE("Generating Makefile for nfCommon")

FILE(GLOB NFCOMMON_SOURCES        *.cpp)
FILE(GLOB NFCOMMON_HEADERS        *.hpp)
FILE(GLOB NFCOMMON_LINUX_SOURCES  Linux/*.cpp)
FILE(GLOB NFCOMMON_LINUX_HEADERS  Linux/*.hpp)
FILE(GLOB NFCOMMON_MATH_SOURCES   Math/*.cpp)
FILE(GLOB NFCOMMON_MATH_HEADERS   Math/*.hpp)
FILE(GLOB NFCOMMON_PACKER_SOURCES Packer/*.cpp)
FILE(GLOB NFCOMMON_PACKER_HEADERS Packer/*.hpp)
FILE(GLOB NFCOMMON_LOGGER_SOURCES LoggerBackends/*.cpp)
FILE(GLOB NFCOMMON_LOGGER_HEADERS LoggerBackends/*.hpp)
FILE(GLOB NFCOMMON_IMAGE_SOURCES  Image/*.cpp)
FILE(GLOB NFCOMMON_IMAGE_HEADERS  Image/*.hpp)

# TODO these sources are not prepared for multi-platform.
#      Move them to Win directory and appropriately code headers.
#      For reference purposes, each line will have a GitHub issue number

LIST(REMOVE_ITEM NFCOMMON_SOURCES "${NFCOMMON_DIRECTORY}/Memory.cpp")       # 105

# Windows DllMain definition
IF(NOT WIN32)
    LIST(REMOVE_ITEM NFCOMMON_SOURCES "${NFCOMMON_DIRECTORY}/Main.cpp")
ENDIF(NOT WIN32)

INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/ ${NFCOMMON_DIRECTORY})

ADD_LIBRARY(nfCommon SHARED
            ${NFCOMMON_SOURCES} ${NFCOMMON_HEADERS}
            ${NFCOMMON_MATH_SOURCES} ${NFCOMMON_MATH_HEADERS}
            ${NFCOMMON_PACKER_SOURCES} ${NFCOMMON_PACKER_HEADERS}
            ${NFCOMMON_LOGGER_SOURCES} ${NFCOMMON_LOGGER_HEADERS}
            ${NFCOMMON_LINUX_SOURCES} ${NFCOMMON_LINUX_HEADERS}
            ${NFCOMMON_IMAGE_SOURCES} ${NFCOMMON_IMAGE_HEADERS})

ADD_CUSTOM_COMMAND(TARGET nfCommon POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfCommon> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
