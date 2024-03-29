# @file
# @author Witek902
# @brief  CMake for RendererCommon

MESSAGE(STATUS "Generating build files for RendererCommon")

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

SET_PROPERTY(TARGET RendererCommon PROPERTY FOLDER Src/Engine/Renderers)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERER_COMMON_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERER_COMMON_HEADERS)
NFE_SOURCE_GROUP_PCH(RendererCommon)
