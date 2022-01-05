# @file
# @author Mkkulagowski (mk.kulagowski(at)gmail.com)
# @brief  CMake for TestCalcLib

MESSAGE("Generating Makefile for testCalcLib")

SET(TESTCALCLIB_SOURCES
    TestCalcLib.cpp
    )

SET(TESTCALCLIB_HEADERS
    TestCalcLib.hpp
    )

# setup directories
LINK_DIRECTORIES(${NFCOMMONTEST_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

# build shared lib
ADD_LIBRARY(testCalcLib SHARED ${TESTCALCLIB_SOURCES} ${TESTCALCLIB_HEADERS})

TARGET_COMPILE_DEFINITIONS(testCalcLib
                           PRIVATE TESTCALCDLL_EXPORTS
                           )

ADD_CUSTOM_COMMAND(TARGET testCalcLib POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:testCalcLib> ${NFE_OUTPUT_DIRECTORY}/${targetfile})


