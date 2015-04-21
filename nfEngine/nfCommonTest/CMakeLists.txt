# @file   CMakeLists.txt
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for nfCommonTest

MESSAGE("Generating Makefile for nfCommonTest")

FILE(GLOB NFCOMMONTEST_SOURCES *.cpp)
FILE(GLOB NFCOMMONTEST_HEADERS *.hpp)

# Search for dependencies
PKG_CHECK_MODULES(NFCOMMONTEST_DEPS REQUIRED
                  x11) # needed by nfCommon Window

# TODO these tests have unfinished linux versions
LIST(REMOVE_ITEM NFCOMMONTEST_SOURCES "${NFCOMMONTEST_DIRECTORY}/PackerBasicTest.cpp")
LIST(REMOVE_ITEM NFCOMMONTEST_SOURCES "${NFCOMMONTEST_DIRECTORY}/PackerComplexTest.cpp")
LIST(REMOVE_ITEM NFCOMMONTEST_SOURCES "${NFCOMMONTEST_DIRECTORY}/MemoryTest.cpp")	 #105

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFCOMMON_DIRECTORY}
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include
                    ${NFECOMMONTEST_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(nfCommonTest ${NFCOMMONTEST_SOURCES} ${NFCOMMONTEST_HEADERS})
SET_TARGET_PROPERTIES(nfCommonTest PROPERTIES
                      COMPILE_FLAGS "-pthread"
                      LINK_FLAGS "-pthread")
ADD_DEPENDENCIES(nfCommonTest nfCommon)
TARGET_LINK_LIBRARIES(nfCommonTest nfCommon gtest ${NFCOMMONTEST_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET nfCommonTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfCommonTest> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
