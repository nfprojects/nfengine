# @file   CMakeLists.txt
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

# TODO these sources are not prepared for multi-platform.
#      Move them to Win directory and appropriately code headers.
#      For reference purposes, each line will have a GitHub issue number

LIST(REMOVE_ITEM NFCOMMON_SOURCES "${NFCOMMON_DIRECTORY}/Logger.cpp")       #   2
LIST(REMOVE_ITEM NFCOMMON_SOURCES "${NFCOMMON_DIRECTORY}/OutputStream.cpp") # 108
LIST(REMOVE_ITEM NFCOMMON_SOURCES "${NFCOMMON_DIRECTORY}/InputStream.cpp")  # 108

# Image.cpp requires Input/OutputStream to work
LIST(REMOVE_ITEM NFCOMMON_SOURCES "${NFCOMMON_DIRECTORY}/Image.cpp")

# Windows DllMain definition
IF(NOT WIN32)
    LIST(REMOVE_ITEM NFCOMMON_SOURCES "${NFCOMMON_DIRECTORY}/Main.cpp")
ENDIF(NOT WIN32)

INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/ ${NFCOMMON_DIRECTORY})

ADD_LIBRARY(nfCommon SHARED
            ${NFCOMMON_SOURCES} ${NFCOMMON_HEADERS}
            ${NFCOMMON_MATH_SOURCES} ${NFCOMMON_MATH_HEADERS}
#            ${NFCOMMON_PACKER_SOURCES} ${NFCOMMON_PACKER_HEADERS})         #  44
            ${NFCOMMON_LINUX_SOURCES} ${NFCOMMON_LINUX_HEADERS})
ADD_CUSTOM_COMMAND(TARGET nfCommon POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfCommon> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
