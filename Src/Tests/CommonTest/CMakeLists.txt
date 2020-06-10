# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for CommonTest

MESSAGE("Generating Makefile for CommonTest")

SET(NFE_COMMON_TEST_DIRECTORY ${NFE_TESTS_DIRECTORY}/CommonTest)

FILE(GLOB NFCOMMONTEST_SOURCES *.cpp)
FILE(GLOB NFCOMMONTEST_HEADERS *.hpp)
FILE(GLOB_RECURSE NFCOMMONTEST_TEST_CASES_SOURCES *.cpp)

# Search for dependencies
PKG_CHECK_MODULES(NFCOMMONTEST_DEPS REQUIRED
                  xcb)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFE_SRC_DIRECTORY}
                    ${NFE_COMMON_TEST_DIRECTORY}
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include
                    ${NFECOMMONTEST_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(CommonTest ${NFCOMMONTEST_SOURCES} ${NFCOMMONTEST_HEADERS} ${NFCOMMONTEST_TEST_CASES_SOURCES})
SET_TARGET_PROPERTIES(CommonTest PROPERTIES
                      LINK_FLAGS "-pthread")
ADD_DEPENDENCIES(CommonTest Common testCalcLib)
TARGET_LINK_LIBRARIES(CommonTest Common gtest dl ${NFCOMMONTEST_DEPS_LIBRARIES})
TARGET_PRECOMPILE_HEADERS(CommonTest PRIVATE PCH.hpp)

ADD_CUSTOM_COMMAND(TARGET CommonTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CommonTest> ${NFE_OUTPUT_DIRECTORY}/${targetfile})

ADD_SUBDIRECTORY("TestResources")
