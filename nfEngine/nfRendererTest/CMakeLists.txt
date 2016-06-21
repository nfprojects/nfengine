# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for nfRendererTest

MESSAGE("Generating Makefile for nfRendererTest")

FILE(GLOB NFRENDERERTEST_SOURCES *.cpp)
FILE(GLOB NFRENDERERTEST_HEADERS *.hpp)

# Search for dependencies
PKG_CHECK_MODULES(NFRENDERERTEST_DEPS REQUIRED
                  x11
                  gl) # needed by nfCommon Window

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFCOMMON_DIRECTORY}
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include
                    ${NFRENDERERTEST_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY}
                 ${NFRENDERERTEST_DEPS_LIBRARY_DIRS})

ADD_EXECUTABLE(nfRendererTest ${NFRENDERERTEST_SOURCES} ${NFRENDERERTEST_HEADERS})
SET_TARGET_PROPERTIES(nfRendererTest PROPERTIES
                      COMPILE_FLAGS "-pthread"
                      LINK_FLAGS "-pthread")

# nfRendererOG4 dependency removed temporarily
ADD_DEPENDENCIES(nfRendererTest nfCommon)
TARGET_LINK_LIBRARIES(nfRendererTest nfCommon gtest png squish jpeg
                      ${NFRENDERERTEST_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET nfRendererTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfRendererTest> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
