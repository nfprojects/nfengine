/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 * @details The interface is communication layer between rendering API (such as Direct3D or OpenGL)
 *          and high-level engine's renderer. The API provides basic abstract classes:
 *          vertex buffers, index buffers, shader constant buffers, renderer states, textures,
 *          rendertargets, etc.
 */

#pragma once

#include "CommandBuffer.hpp"

#define RENDERER_INIT_FUNC "Init"

namespace NFE {
namespace Renderer {

/**
 * Rendering device interface.
 *
 * @details Interface allowing to create GPU resources and manage command buffers.
 */
class IDevice
{
public:
    virtual ~IDevice() {}

    /**
     * Get handle to a main rendering API object.
     */
    virtual void* GetHandle() const = 0;

    /**
     * DeviceResourcesCreation Resources creation functions
     * @{
     */

    virtual IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc) = 0;
    virtual IBuffer* CreateBuffer(const BufferDesc& desc) = 0;
    virtual ITexture* CreateTexture(const TextureDesc& desc) = 0;
    virtual IBackbuffer* CreateBackbuffer(const BackbufferDesc& desc) = 0;
    virtual IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc) = 0;
    virtual IBlendState* CreateBlendState(const BlendStateDesc& desc) = 0;
    virtual IDepthState* CreateDepthState(const DepthStateDesc& desc) = 0;
    virtual IRasterizerState* CreateRasterizerState(const RasterizerStateDesc& desc) = 0;
    virtual ISampler* CreateSampler(const SamplerDesc& desc) = 0;
    virtual IShader* CreateShader(const ShaderDesc& desc) = 0;
    virtual IShaderProgram* CreateShaderProgram(const ShaderProgramDesc& desc) = 0;

    /**@}*/

    virtual ICommandBuffer* GetDefaultCommandBuffer() = 0;
    virtual void Execute(ICommandBuffer* commandBuffer, bool saveState) = 0;
};

typedef IDevice* (*RendererInitFunc)();
typedef void (*RendererReleaseFunc)();

} // namespace Renderer
} // namespace NFE
