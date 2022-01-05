# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for RendererTest

MESSAGE(STATUS "Generating build files for RendererTest")

SET(NFE_RENDERER_TEST_DIRECTORY ${NFE_TESTS_DIRECTORY}/RendererTest)

SET(NFRENDERERTEST_SOURCES
    PCH.cpp
    Main.cpp
    Backends.cpp
    DrawTest.cpp
    RendererTest.cpp
    TestCases/Buffer.cpp
    TestCases/CommandBuffer.cpp
    TestCases/ResourceBinding.cpp
    TestCases/Shader.cpp
    TestCases/SimpleDraw.cpp
    TestCases/Texture2D.cpp
)

SET(NFRENDERERTEST_HEADERS
    PCH.hpp
    Backends.hpp
    DrawTest.hpp
    RendererTest.hpp
)

IF(UNIX)
    # Search for dependencies
    PKG_CHECK_MODULES(NFRENDERERTEST_DEPS REQUIRED
        xcb
    )
ENDIF(UNIX)

ADD_EXECUTABLE(RendererTest ${NFRENDERERTEST_SOURCES} ${NFRENDERERTEST_HEADERS})

TARGET_INCLUDE_DIRECTORIES(RendererTest
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
    PRIVATE ${NFE_SRC_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/googletest/googletest/include
    PRIVATE ${NFRENDERERTEST_DEPS_INCLUDE_DIRS}
)

TARGET_LINK_DIRECTORIES(RendererTest
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
    PRIVATE ${NFE_OUTPUT_DIRECTORY}
    PRIVATE ${NFRENDERERTEST_DEPS_LIBRARY_DIRS}
)

IF(UNIX)
    SET_TARGET_PROPERTIES(RendererTest PROPERTIES LINK_FLAGS "-pthread")
ENDIF(UNIX)

ADD_DEPENDENCIES(RendererTest RendererCommon Common)
TARGET_LINK_LIBRARIES(RendererTest RendererCommon Common gtest ${NFRENDERERTEST_DEPS_LIBRARIES})

TARGET_PRECOMPILE_HEADERS(RendererTest PRIVATE PCH.hpp)

ADD_CUSTOM_COMMAND(TARGET RendererTest POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RendererTest> ${NFE_OUTPUT_DIRECTORY}
)
