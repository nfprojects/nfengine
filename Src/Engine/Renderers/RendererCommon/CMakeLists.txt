# @file
# @author Witek902
# @brief  CMake for RendererCommon

MESSAGE("Generating Makefile for RendererCommon")

SET(NFE_RENDERER_COMMON_DIRECTORY ${NFE_RENDERERS_DIRECTORY}/RendererCommon)

SET(NFRENDERER_COMMON_SOURCES
    PCH.cpp
    Fence.cpp
    Types.cpp
    )

SET(NFRENDERER_COMMON_HEADERS
    Backbuffer.hpp
    Buffer.hpp
    CommandQueue.hpp
    CommandRecorder.hpp
    ComputePipelineState.hpp
    Device.hpp
    Fence.hpp
    MemoryBlock.hpp
    PCH.hpp
    PipelineState.hpp
    RendererCommon.hpp
    RenderTarget.hpp
    ResourceBinding.hpp
    Shader.hpp
    Texture.hpp
    Types.hpp
    VertexLayout.hpp
    )

ADD_LIBRARY(RendererCommon SHARED ${NFRENDERER_COMMON_SOURCES} ${NFRENDERER_COMMON_HEADERS})

TARGET_COMPILE_DEFINITIONS(RendererCommon
                           PRIVATE NFE_RENDERER_COMMON_EXPORTS
                           )

TARGET_INCLUDE_DIRECTORIES(RendererCommon
                           PRIVATE ${NFE_RENDERER_COMMON_DIRECTORY}
                           PRIVATE ${NFE_SRC_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
                           )

TARGET_LINK_DIRECTORIES(RendererCommon PRIVATE ${NFEDEPS_LIB_DIRECTORY})

ADD_DEPENDENCIES(RendererCommon Common)

TARGET_LINK_LIBRARIES(RendererCommon Common)

TARGET_PRECOMPILE_HEADERS(RendererCommon PRIVATE PCH.hpp)

ADD_CUSTOM_COMMAND(TARGET RendererCommon POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RendererCommon> ${NFE_OUTPUT_DIRECTORY}/${targetfile})

