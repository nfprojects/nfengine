# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for RendererVk

MESSAGE("Generating Makefile for RendererVk")

SET(NFE_RENDERERVK_DIRECTORY ${NFE_RENDERERS_DIRECTORY}/RendererVk)

SET(NFRENDERERVK_SOURCES
    API/Backbuffer.cpp
    API/Buffer.cpp
    API/CommandRecorder.cpp
    API/ComputePipelineState.cpp
    API/Device.cpp
    API/PipelineState.cpp
    API/RenderTarget.cpp
    API/ResourceBinding.cpp
    API/Sampler.cpp
    API/Shader.cpp
    API/Texture.cpp
    API/VertexLayout.cpp
    Internal/Debugger.cpp
    Internal/Extensions.cpp
    Internal/Instance.cpp
    Internal/RenderPassManager.cpp
    Internal/RingBuffer.cpp
    # Internal/SemaphorePool.cpp temporarily commented out
    Internal/ShaderIncluder.cpp
    Internal/Translations.cpp
    )

SET(NFRENDERERVK_HEADERS
    API/Backbuffer.hpp
    API/Buffer.hpp
    API/CommandRecorder.hpp
    API/ComputePipelineState.hpp
    API/Device.hpp
    API/IResource.hpp
    API/PipelineState.hpp
    API/RenderTarget.hpp
    API/ResourceBinding.hpp
    API/Sampler.hpp
    API/Shader.hpp
    API/Texture.hpp
    API/VertexLayout.hpp
    Internal/Debugger.hpp
    Internal/Extensions.hpp
    Internal/GetProc.hpp
    Internal/Instance.hpp
    Internal/RenderPassDesc.hpp
    Internal/RenderPassManager.hpp
    Internal/RingBuffer.hpp
    # Internal/SemaphorePool.hpp temporarily commented out
    Internal/ShaderIncluder.hpp
    Internal/Translations.hpp
    )

SET(NFRENDERERVK_LINUX_SOURCES
    API/Linux/XcbBackbuffer.cpp
    API/Linux/XcbDevice.cpp
    Internal/Linux/XcbExtensions.cpp
    )

SET(NFRENDERERVK_LINUX_HEADERS
    Internal/Linux/XcbExtensions.hpp
    )

# GLSLang's default resource limits definition
SET(NFRENDERERVK_GLSLANG_DEFAULT_RESOURCE_LIMITS
    ${NFEDEPS_ROOT_DIRECTORY}/glslang/StandAlone/ResourceLimits.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/glslang/StandAlone/ResourceLimits.h
    )

# Skips Main.cpp on purpose, because there's only Main.cpp definition for Win32
SET(NFRENDERERVK_MAIN_SOURCES
    PCH.cpp
    )

SET(NFRENDERERVK_MAIN_HEADERS
    PCH.hpp
    )

ADD_LIBRARY(RendererVk SHARED
            ${NFRENDERERVK_SOURCES} ${NFRENDERERVK_HEADERS}
            ${NFRENDERERVK_LINUX_SOURCES} ${NFRENDERERVK_LINUX_HEADERS}
            ${NFRENDERERVK_MAIN_SOURCES} ${NFRENDERERVK_MAIN_HEADERS}
            ${NFRENDERERVK_GLSLANG_DEFAULT_RESOURCE_LIMITS}
            )

# To enable XCB-specific Vulkan extensions
TARGET_COMPILE_DEFINITIONS(RendererVk
                           PRIVATE "-DVK_USE_PLATFORM_XCB_KHR"
)

TARGET_INCLUDE_DIRECTORIES(RendererVk
                           PRIVATE ${NFE_RENDERERVK_DIRECTORY}
                           PRIVATE ${NFE_SRC_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
                           PRIVATE ${VULKAN_SDK}/include
)

TARGET_LINK_DIRECTORIES(RendererVk
                        PRIVATE ${NFEDEPS_LIB_DIRECTORY}
)

ADD_DEPENDENCIES(RendererVk RendererCommon)

TARGET_LINK_LIBRARIES(RendererVk RendererCommon vulkan glslang HLSL SPIRV SPIRV-Tools SPIRV-Tools-opt OGLCompiler OSDependent)

TARGET_PRECOMPILE_HEADERS(RendererVk PRIVATE PCH.hpp)

ADD_CUSTOM_COMMAND(TARGET RendererVk POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RendererVk> ${NFE_OUTPUT_DIRECTORY}/${targetfile})

