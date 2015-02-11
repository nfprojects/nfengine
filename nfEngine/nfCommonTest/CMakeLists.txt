# @file   CMakeLists.txt
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for nfCommonTest

MESSAGE("Generating Makefile for nfCommonTest")

FILE(GLOB NFCOMMONTEST_SOURCES *.cpp)
FILE(GLOB NFCOMMONTEST_HEADERS *.hpp)

# TODO these tests have unfinished linux versions
LIST(REMOVE_ITEM NFCOMMONTEST_SOURCES "${NFCOMMONTEST_DIRECTORY}/TimerTest.cpp")
LIST(REMOVE_ITEM NFCOMMONTEST_SOURCES "${NFCOMMONTEST_DIRECTORY}/PackerBasicTest.cpp")
LIST(REMOVE_ITEM NFCOMMONTEST_SOURCES "${NFCOMMONTEST_DIRECTORY}/PackerComplexTest.cpp")
# ThreadPoolPerformance uses Timer to measure time - delete below when Timer will be fixed
LIST(REMOVE_ITEM NFCOMMONTEST_SOURCES "${NFCOMMONTEST_DIRECTORY}/ThreadPoolPerformance.cpp")

INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFCOMMON_DIRECTORY}
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include)
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(nfCommonTest ${NFCOMMONTEST_SOURCES} ${NFCOMMONTEST_HEADERS})
SET_TARGET_PROPERTIES(nfCommonTest PROPERTIES
                      COMPILE_FLAGS "-pthread"
                      LINK_FLAGS "-pthread")
ADD_DEPENDENCIES(nfCommonTest nfCommon)
TARGET_LINK_LIBRARIES(nfCommonTest nfCommon gtest)
ADD_CUSTOM_COMMAND(TARGET nfCommonTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfCommonTest> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
