# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for RendererVk

MESSAGE(STATUS "Generating build files for RendererVk")

MESSAGE(STATUS "RendererVk - Vulkan SDK Path: $ENV{VULKAN_SDK}")

IF(NOT EXISTS $ENV{VULKAN_SDK})
    MESSAGE(FATAL_ERROR "Path to Vulkan SDK not provided or invalid - specify it via an env variable VULKAN_SDK.")
ENDIF(NOT EXISTS $ENV{VULKAN_SDK})

SET(NFE_RENDERERVK_DIRECTORY ${NFE_RENDERERS_DIRECTORY}/RendererVk)

SET(NFRENDERERVK_SOURCES
    PCH.cpp
    API/Backbuffer.cpp
    API/Buffer.cpp
    API/CommandList.cpp
    API/CommandQueue.cpp
    API/CommandRecorder.cpp
    API/ComputePipelineState.cpp
    API/Device.cpp
    API/Fence.cpp
    API/MemoryBlock.cpp
    API/PipelineState.cpp
    API/RenderTarget.cpp
    API/Sampler.cpp
    API/Shader.cpp
    API/Texture.cpp
    API/VertexLayout.cpp
    Internal/CommandBufferManager.cpp
    Internal/Debugger.cpp
    Internal/Extensions.cpp
    Internal/FenceSignaller.cpp
    Internal/Instance.cpp
    Internal/LayoutTracker.cpp
    Internal/QueueFamilyManager.cpp
    Internal/RenderPassManager.cpp
    Internal/RingBuffer.cpp
    # Internal/SemaphorePool.cpp temporarily commented out
    Internal/ShaderIncluder.cpp
    Internal/Translations.cpp
    Internal/Utilities.cpp
)

SET(NFRENDERERVK_HEADERS
    Defines.hpp
    API/Backbuffer.hpp
    API/Buffer.hpp
    API/CommandList.hpp
    API/CommandQueue.hpp
    API/CommandRecorder.hpp
    API/ComputePipelineState.hpp
    API/Device.hpp
    API/Fence.hpp
    API/IResource.hpp
    API/MemoryBlock.hpp
    API/PipelineState.hpp
    API/RenderTarget.hpp
    API/Sampler.hpp
    API/Shader.hpp
    API/Texture.hpp
    API/VertexLayout.hpp
    Internal/CommandBufferManager.hpp
    Internal/Debugger.hpp
    Internal/Extensions.hpp
    Internal/FenceSignaller.hpp
    Internal/GetProc.hpp
    Internal/Instance.hpp
    Internal/LayoutTracker.hpp
    Internal/QueueFamilyManager.hpp
    Internal/RenderPassDesc.hpp
    Internal/RenderPassManager.hpp
    Internal/RingBuffer.hpp
    # Internal/SemaphorePool.hpp temporarily commented out
    Internal/ShaderIncluder.hpp
    Internal/Translations.hpp
    Internal/Types.hpp
    Internal/Utilities.hpp
)

IF(WIN32)
    SET(NFRENDERERVK_SOURCES ${NFRENDERERVK_SOURCES}
        Main.cpp
        API/Win/WinBackbuffer.cpp
        API/Win/WinDevice.cpp
        Internal/Win/WinExtensions.cpp
    )

    SET(NFRENDERERVK_HEADERS ${NFRENDERERVK_HEADERS}
        Internal/Win/WinExtensions.hpp
    )
ELSEIF(UNIX)
    SET(NFRENDERERVK_SOURCES ${NFRENDERERVK_SOURCES}
        API/Linux/XcbBackbuffer.cpp
        API/Linux/XcbDevice.cpp
        Internal/Linux/XcbExtensions.cpp
    )

    SET(NFRENDERERVK_HEADERS ${NFRENDERERVK_HEADERS}
        Internal/Linux/XcbExtensions.hpp
    )
ELSE(WIN32)
    MESSAGE(FATAL_ERROR "Target platform not supported")
ENDIF(WIN32)

ADD_LIBRARY(RendererVk SHARED ${NFRENDERERVK_SOURCES} ${NFRENDERERVK_HEADERS})

TARGET_COMPILE_DEFINITIONS(RendererVk
    PRIVATE RENDERERVK_EXPORTS
)

# To enable platform-specific Vulkan extensions
IF(WIN32)
    TARGET_COMPILE_DEFINITIONS(RendererVk PRIVATE VK_USE_PLATFORM_WIN32_KHR)
ELSEIF(UNIX)
    # Linux build uses XCB for window system communication
    TARGET_COMPILE_DEFINITIONS(RendererVk PRIVATE VK_USE_PLATFORM_XCB_KHR)
ELSE(WIN32)
    MESSAGE(FATAL_ERROR "Target platform not supported")
ENDIF(WIN32)

TARGET_INCLUDE_DIRECTORIES(RendererVk
    PRIVATE ${NFE_RENDERERVK_DIRECTORY}
    PRIVATE ${NFE_SRC_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
    PRIVATE $ENV{VULKAN_SDK}/include
)

TARGET_LINK_DIRECTORIES(RendererVk
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
    PRIVATE $ENV{VULKAN_SDK}/lib
)

ADD_DEPENDENCIES(RendererVk RendererCommon glslang glslang-default-resource-limits HLSL SPIRV spirv-reflect-static)

IF(WIN32)
    SET(NFRENDERERVK_VULKAN_LIB vulkan-1)
ELSEIF(UNIX)
    SET(NFRENDERERVK_VULKAN_LIB vulkan)
ELSE(WIN32)
    MESSAGE(FATAL_ERROR "Target platform not supported")
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(RendererVk
    RendererCommon ${NFRENDERERVK_VULKAN_LIB} glslang glslang-default-resource-limits
    HLSL SPIRV SPIRV-Tools SPIRV-Tools-opt OGLCompiler OSDependent
    spirv-reflect-static
)

TARGET_PRECOMPILE_HEADERS(RendererVk PRIVATE PCH.hpp)

SET_PROPERTY(TARGET RendererVk PROPERTY FOLDER Src/Engine/Renderers)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERERVK_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(NFRENDERERVK_HEADERS)
NFE_SOURCE_GROUP_PCH(RendererVk)
