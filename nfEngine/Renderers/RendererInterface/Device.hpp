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
#define RENDERER_RELEASE_FUNC "Release"

namespace NFE {
namespace Renderer {

/**
 * Rendering device information.
 */
struct DeviceInfo
{
    std::string description;  //< GPU name
    std::string misc;         //< miscellaneous GPU info
    std::vector<std::string> features; //< list of supported fetures (depends on low-level API)
};

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
     * Get rendering device information.
     * @param info Reference to DeviceInfo object, where information will be stored
     * @return True on success.
     */
    virtual bool GetDeviceInfo(DeviceInfo& info) = 0;

    /**
     * DeviceResourcesCreation Resources creation functions
     * @{
     */

    virtual IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc) = 0;
    virtual IBuffer* CreateBuffer(const BufferDesc& desc) = 0;
    virtual ITexture* CreateTexture(const TextureDesc& desc) = 0;
    virtual IBackbuffer* CreateBackbuffer(const BackbufferDesc& desc) = 0;
    virtual IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc) = 0;
    virtual IPipelineState* CreatePipelineState(const PipelineStateDesc& desc) = 0;
    virtual ISampler* CreateSampler(const SamplerDesc& desc) = 0;
    virtual IShader* CreateShader(const ShaderDesc& desc) = 0;
    virtual IShaderProgram* CreateShaderProgram(const ShaderProgramDesc& desc) = 0;

    /**@}*/

    virtual ICommandBuffer* GetDefaultCommandBuffer() = 0;

    /**
     * Create a deffered context.
     */
    virtual ICommandBuffer* CreateCommandBuffer() = 0;
};

typedef IDevice* (*RendererInitFunc)();
typedef void (*RendererReleaseFunc)();

} // namespace Renderer
} // namespace NFE
