# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for RendererDemo

MESSAGE(STATUS "Generating build files for RendererDemo")

SET(NFE_RENDERER_DEMO_DIRECTORY ${NFE_APPS_DIRECTORY}/RendererDemo)

SET(NFRENDERERDEMO_SOURCES
    PCH.cpp
    Main.cpp
    Common.cpp
    Scenes/Basic.cpp
    Scenes/Compute.cpp
    Scenes/DepthStencil.cpp
    Scenes/DynamicTexture.cpp
    Scenes/Multisample.cpp
    Scenes/Multithreaded.cpp
    Scenes/RenderTargets.cpp
    Scenes/Scene.cpp
    Scenes/Tessellation.cpp
    Scenes/VertexBuffers.cpp
)

SET(NFRENDERERDEMO_HEADERS
    PCH.hpp
    Common.hpp
    Scenes/Scene.hpp
)

IF(UNIX)
    # Search for dependencies
    PKG_CHECK_MODULES(NFRENDERERDEMO_DEPS REQUIRED
        xcb
    )
ENDIF(UNIX)

ADD_EXECUTABLE(RendererDemo ${NFRENDERERDEMO_SOURCES} ${NFRENDERERDEMO_HEADERS})

TARGET_INCLUDE_DIRECTORIES(RendererDemo
    PRIVATE ${NFE_RENDERER_DEMO_DIRECTORY}
    PRIVATE ${NFE_SRC_DIRECTORY}
    PRIVATE ${NFRENDERERDEMO_DEPS_INCLUDE_DIRS}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
)

TARGET_LINK_DIRECTORIES(RendererDemo
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
    PRIVATE ${NFE_OUTPUT_DIRECTORY}
)

IF(UNIX)
    SET_TARGET_PROPERTIES(RendererDemo PROPERTIES LINK_FLAGS "-pthread")
ENDIF(UNIX)

TARGET_PRECOMPILE_HEADERS(RendererDemo PRIVATE PCH.hpp)

ADD_DEPENDENCIES(RendererDemo Common RendererCommon RendererVk)
IF(WIN32)
    ADD_DEPENDENCIES(RendererDemo RendererD3D12)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(RendererDemo Common RendererCommon ${NFRENDERERDEMO_DEPS_LIBRARIES})

ADD_CUSTOM_COMMAND(TARGET RendererDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RendererDemo> ${NFE_OUTPUT_DIRECTORY}
)

SET_PROPERTY(TARGET RendererDemo PROPERTY FOLDER Src/Apps)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERERDEMO_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERERDEMO_HEADERS)
NFE_SOURCE_GROUP_PCH(RendererDemo)
