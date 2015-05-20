# @file   CMakeLists.txt
# @author Mkkulagowski (mk.kulagowski(at)gmail.com)
# @brief  CMake for testCalcLib

MESSAGE("Generating Makefile for testCalcLib")

# setup directories
LINK_DIRECTORIES(${NFCOMMONTEST_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

# build shared lib
ADD_LIBRARY(testCalcLib SHARED "${NFCOMMONTEST_DIRECTORY}/TestCalcLib/TestCalcLib.cpp"
                                "${NFCOMMONTEST_DIRECTORY}/TestCalcLib/TestCalcLib.hpp")

ADD_CUSTOM_COMMAND(TARGET testCalcLib POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:testCalcLib> ${NFE_OUTPUT_DIRECTORY}/${targetfile})


