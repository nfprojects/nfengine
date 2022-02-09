# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for RendererD3D12

MESSAGE(STATUS "Generating build files for RendererD3D12")

SET(NFE_RENDERERD3D12_DIRECTORY ${NFE_RENDERERS_DIRECTORY}/RendererD3D12)

SET(NFRENDERERD3D12_SOURCES
    PCH.cpp
    Main.cpp
    Backbuffer.cpp
    Buffer.cpp
    CommandList.cpp
    CommandListManager.cpp
    CommandQueue.cpp
    CommandRecorder.cpp
    Common.cpp
    ComputePipelineState.cpp
    Device.cpp
    Fence.cpp
    HeapAllocator.cpp
    MemoryBlock.cpp
    PipelineState.cpp
    ReferencedResourcesList.cpp
    RendererD3D12.cpp
    RenderTarget.cpp
    Resource.cpp
    ResourceStateCache.cpp
    ResourceState.cpp
    RingBuffer.cpp
    Sampler.cpp
    ShaderCompiler.cpp
    Shader.cpp
    Texture.cpp
    Translations.cpp
    VertexLayout.cpp
)

SET(NFRENDERERD3D12_HEADERS
    PCH.hpp
    Backbuffer.hpp
    Buffer.hpp
    CommandList.hpp
    CommandListManager.hpp
    CommandQueue.hpp
    CommandRecorder.hpp
    Common.hpp
    ComputePipelineState.hpp
    D3DPtr.hpp
    Device.hpp
    Fence.hpp
    Format.hpp
    HeapAllocator.hpp
    MemoryBlock.hpp
    PipelineState.hpp
    ReferencedResourcesList.hpp
    RendererD3D12.hpp
    RenderTarget.hpp
    Resource.hpp
    ResourceStateCache.hpp
    ResourceState.hpp
    RingBuffer.hpp
    Sampler.hpp
    ShaderCompiler.hpp
    Shader.hpp
    Texture.hpp
    Translations.hpp
    VertexLayout.hpp
)

ADD_LIBRARY(RendererD3D12 SHARED ${NFRENDERERD3D12_SOURCES} ${NFRENDERERD3D12_HEADERS})

TARGET_COMPILE_DEFINITIONS(RendererD3D12
    PRIVATE RENDERERD3D12_EXPORTS
)

TARGET_INCLUDE_DIRECTORIES(RendererD3D12
    PRIVATE ${NFE_RENDERERVK_DIRECTORY}
    PRIVATE ${NFE_SRC_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/D3D12MemoryAllocator/include
)

TARGET_LINK_DIRECTORIES(RendererD3D12
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
)

ADD_DEPENDENCIES(RendererD3D12 RendererCommon D3D12MemoryAllocator)

TARGET_LINK_LIBRARIES(RendererD3D12 RendererCommon D3D12MemoryAllocator d3d12 dxgi dxguid dxcompiler)

TARGET_PRECOMPILE_HEADERS(RendererD3D12 PRIVATE PCH.hpp)

SET_PROPERTY(TARGET RendererD3D12 PROPERTY FOLDER Src/Engine/Renderers)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERERD3D12_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERERD3D12_HEADERS)
NFE_SOURCE_GROUP_PCH(RendererD3D12)
