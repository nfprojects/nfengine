# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for RendererTest

MESSAGE("Generating Makefile for RendererTest")

SET(NFE_RENDERER_TEST_DIRECTORY ${NFE_TESTS_DIRECTORY}/RendererTest)

FILE(GLOB NFRENDERERTEST_SOURCES *.cpp)
FILE(GLOB NFRENDERERTEST_HEADERS *.hpp)

# Search for dependencies
PKG_CHECK_MODULES(NFRENDERERTEST_DEPS REQUIRED
                  xcb)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFE_SRC_DIRECTORY}
                    ${NFE_ROOT_DIRECTORY}/nfEngineDeps/gtest/include
                    ${NFRENDERERTEST_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY}
                 ${NFRENDERERTEST_DEPS_LIBRARY_DIRS})

ADD_EXECUTABLE(RendererTest ${NFRENDERERTEST_SOURCES} ${NFRENDERERTEST_HEADERS})
SET_TARGET_PROPERTIES(RendererTest PROPERTIES
                      LINK_FLAGS "-pthread")

ADD_DEPENDENCIES(RendererTest Common)
TARGET_LINK_LIBRARIES(RendererTest Common gtest dl ${NFRENDERERTEST_DEPS_LIBRARIES})

TARGET_PRECOMPILE_HEADERS(RendererTest PRIVATE PCH.hpp)

ADD_CUSTOM_COMMAND(TARGET RendererTest POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RendererTest> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
