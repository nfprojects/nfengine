# @file
# @author Mkkulagowski (mk.kulagowski(at)gmail.com)
# @brief  CMake for TestCalcLib

MESSAGE("Generating Makefile for testCalcLib")

FILE(GLOB TESTCALCLIB_SOURCES *.cpp)
FILE(GLOB TESTCALCLIB_HEADERS *.hpp)

# setup directories
LINK_DIRECTORIES(${NFCOMMONTEST_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

# build shared lib
ADD_LIBRARY(testCalcLib SHARED ${TESTCALCLIB_SOURCES} ${TESTCALCLIB_HEADERS})

ADD_CUSTOM_COMMAND(TARGET testCalcLib POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:testCalcLib> ${NFE_OUTPUT_DIRECTORY}/${targetfile})


