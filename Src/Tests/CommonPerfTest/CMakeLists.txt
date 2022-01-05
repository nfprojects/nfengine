# @file
# @author Witek902 (witek902@gmail.com)
# @brief  CMake for CommonPerfTest

MESSAGE("Generating Makefile for CommonPerfTest")

SET(NFE_COMMON_PERF_TEST_DIRECTORY ${NFE_TESTS_DIRECTORY}/CommonPerfTest)

SET(NFCOMMONPERFTEST_SOURCES
    PCH.cpp
    Main.cpp
    )

SET(NFCOMMONPERFTEST_HEADERS
    PCH.hpp
    Test.hpp
    )

SET(NFCOMMONPERFTEST_TEST_CASES_SOURCES
    TestCases/BVHPerfTest.cpp
    TestCases/ConfigPerfTest.cpp
    TestCases/FilePerfTest.cpp
    TestCases/HashSetPerfTest.cpp
    TestCases/LoggerPerfTest.cpp
    TestCases/SetPerfTest.cpp
    TestCases/ThreadPoolPerfTest.cpp
    )

IF(UNIX)
    # Search for dependencies
    PKG_CHECK_MODULES(NFCOMMONPERFTEST_DEPS REQUIRED
                      xcb)
ENDIF(UNIX)

# setup directories
ADD_EXECUTABLE(CommonPerfTest
               ${NFCOMMONPERFTEST_SOURCES} ${NFCOMMONPERFTEST_HEADERS} ${NFCOMMONPERFTEST_TEST_CASES_SOURCES})

IF(UNIX)
    SET_TARGET_PROPERTIES(CommonPerfTest PROPERTIES
                          LINK_FLAGS "-pthread")
ENDIF(UNIX)

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

IF(WIN32)
    SET(NFCOMMONPERFTEST_SYSTEM_DEPS )
ELSEIF(UNIX)
    SET(NFCOMMONPERFTEST_SYSTEM_DEPS dl)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(CommonPerfTest Common gtest ${NFCOMMONPERFTEST_SYSTEM_DEPS} ${NFCOMMONPERFTEST_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET CommonPerfTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CommonPerfTest>
                                            ${NFE_OUTPUT_DIRECTORY}/${targetfile})
