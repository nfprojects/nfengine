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
ADD_EXECUTABLE(CommonPerfTest
               ${NFCOMMONPERFTEST_SOURCES} ${NFCOMMONPERFTEST_HEADERS} ${NFCOMMONPERFTEST_TEST_CASES_SOURCES})

SET_TARGET_PROPERTIES(CommonPerfTest PROPERTIES
                      LINK_FLAGS "-pthread")

TARGET_INCLUDE_DIRECTORIES(CommonPerfTest
                           PRIVATE ${NFE_COMMON_PERF_TEST_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/googletest/googletest/include
                           PRIVATE ${NFE_SRC_DIRECTORY}
                           PRIVATE ${NFECOMMONPERFTEST_DEPS_INCLUDE_DIRS})

TARGET_LINK_DIRECTORIES(CommonPerfTest
                        PRIVATE ${NFEDEPS_LIB_DIRECTORY}
                        PRIVATE ${NFE_OUTPUT_DIRECTORY})


TARGET_PRECOMPILE_HEADERS(CommonPerfTest PRIVATE PCH.hpp)

ADD_DEPENDENCIES(CommonPerfTest Common)
TARGET_LINK_LIBRARIES(CommonPerfTest Common gtest dl ${NFCOMMONPERFTEST_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET CommonPerfTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CommonPerfTest>
                                            ${NFE_OUTPUT_DIRECTORY}/${targetfile})
