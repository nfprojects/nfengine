# @file
# @author Witek902 (witek902@gmail.com)
# @brief  CMake for CommonPerfTest

MESSAGE("Generating Makefile for CommonPerfTest")

SET(NFE_COMMON_PERF_TEST_DIRECTORY ${NFE_TESTS_DIRECTORY}/CommonPerfTest)

FILE(GLOB NFCOMMONPERFTEST_SOURCES *.cpp)
FILE(GLOB NFCOMMONPERFTEST_HEADERS *.hpp)
FILE(GLOB NFCOMMONPERFTEST_TEST_CASES_SOURCES TestCases/*.cpp)

# Search for dependencies
PKG_CHECK_MODULES(NFCOMMONPERFTEST_DEPS REQUIRED
                  xcb)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFE_COMMON_PERF_TEST_DIRECTORY}
                    ${NFE_SRC_DIRECTORY}
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include
                    ${NFECOMMONPERFTEST_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(CommonPerfTest
               ${NFCOMMONPERFTEST_SOURCES} ${NFCOMMONPERFTEST_HEADERS} ${NFCOMMONPERFTEST_TEST_CASES_SOURCES})
SET_TARGET_PROPERTIES(CommonPerfTest PROPERTIES
                      LINK_FLAGS "-pthread")

ADD_DEPENDENCIES(CommonPerfTest Common)
TARGET_LINK_LIBRARIES(CommonPerfTest Common gtest dl ${NFCOMMONPERFTEST_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET CommonPerfTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CommonPerfTest>
                                            ${NFE_OUTPUT_DIRECTORY}/${targetfile})
