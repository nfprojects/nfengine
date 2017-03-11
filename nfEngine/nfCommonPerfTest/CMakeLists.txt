# @file
# @author Witek902 (witek902@gmail.com)
# @brief  CMake for nfCommonPerfTest

MESSAGE("Generating Makefile for nfCommonPerfTest")

FILE(GLOB NFCOMMONPERFTEST_SOURCES *.cpp)
FILE(GLOB NFCOMMONPERFTEST_HEADERS *.hpp)
FILE(GLOB NFCOMMONPERFTEST_TEST_CASES_SOURCES TestCases/*.cpp)

# Search for dependencies
PKG_CHECK_MODULES(NFCOMMONPERFTEST_DEPS REQUIRED
                  xcb)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFCOMMONPERFTEST_DIRECTORY}
                    ${NFE_ROOT_DIRECTORY}/nfEngine/
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include
                    ${NFECOMMONPERFTEST_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(nfCommonPerfTest
               ${NFCOMMONPERFTEST_SOURCES} ${NFCOMMONPERFTEST_HEADERS} ${NFCOMMONPERFTEST_TEST_CASES_SOURCES})
SET_TARGET_PROPERTIES(nfCommonPerfTest PROPERTIES
                      LINK_FLAGS "-pthread"
                      COTIRE_CXX_PREFIX_HEADER_INIT "PCH.hpp")

ADD_DEPENDENCIES(nfCommonPerfTest nfCommon)
TARGET_LINK_LIBRARIES(nfCommonPerfTest nfCommon gtest squish jpeg png dl ${NFCOMMONPERFTEST_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET nfCommonPerfTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfCommonPerfTest>
                                            ${NFE_OUTPUT_DIRECTORY}/${targetfile})

cotire(nfCommonPerfTest)
