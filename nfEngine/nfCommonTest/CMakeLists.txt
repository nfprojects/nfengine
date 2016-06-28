# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for nfCommonTest

MESSAGE("Generating Makefile for nfCommonTest")

FILE(GLOB NFCOMMONTEST_SOURCES *.cpp)
FILE(GLOB NFCOMMONTEST_HEADERS *.hpp)

# Search for dependencies
PKG_CHECK_MODULES(NFCOMMONTEST_DEPS REQUIRED
                  x11)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFE_ROOT_DIRECTORY}/nfEngine/
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include
                    ${NFECOMMONTEST_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(nfCommonTest ${NFCOMMONTEST_SOURCES} ${NFCOMMONTEST_HEADERS})
SET_TARGET_PROPERTIES(nfCommonTest PROPERTIES
                      COMPILE_FLAGS "-pthread -lrt"
                      LINK_FLAGS "-pthread -lrt")
ADD_DEPENDENCIES(nfCommonTest nfCommon testCalcLib)
TARGET_LINK_LIBRARIES(nfCommonTest nfCommon gtest squish jpeg png ${NFCOMMONTEST_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET nfCommonTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfCommonTest> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
