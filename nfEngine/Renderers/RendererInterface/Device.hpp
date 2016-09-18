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

#include <vector>


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
     * Check if a given format is supported for a backbuffer texture.
     * @return True if format is supported.
     */
    virtual bool IsBackbufferFormatSupported(ElementFormat format) = 0;

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
    virtual IComputePipelineState* CreateComputePipelineState(const ComputePipelineStateDesc& desc) = 0;
    virtual ISampler* CreateSampler(const SamplerDesc& desc) = 0;
    virtual IShader* CreateShader(const ShaderDesc& desc) = 0;
    virtual IResourceBindingSet* CreateResourceBindingSet(const ResourceBindingSetDesc& desc) = 0;
    virtual IResourceBindingLayout* CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc) = 0;
    virtual IResourceBindingInstance* CreateResourceBindingInstance(IResourceBindingSet* set) = 0;

    /**@}*/

    /**
     * Create a deffered context.
     */
    virtual ICommandBuffer* CreateCommandBuffer() = 0;

    /**
     * Waits until all operations sent to the command queue has been completed.
     * @return True on success.
     */
    virtual bool WaitForGPU() = 0;

    /**
     * Execute a command list.
     * @param commandList Command list to be executed.
     * @return True on success.
     */
    virtual bool Execute(ICommandList* commandList) = 0;

    /**
     * Read data from a GPU buffer to the CPU memory.
     * @param      buffer Source buffer.
     * @param      offset Offset in the GPU buffer (in bytes).
     * @param      size   Number of bytes to read.
     * @param[out] data   Pointer to target CPU buffer.
     * @return true on success.
     */
    virtual bool DownloadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data) = 0;

    /**
     * Read texture content to a CPU buffer.
     * @param      tex  Texture to read.
     * @param[out] data Target CPU buffer.
     */
    virtual bool DownloadTexture(ITexture* tex, void* data, int mipmap = 0, int layer = 0) = 0;
};


/**
 * Structure containing rendering device initialization parameters.
 */
struct DeviceInitParams
{
    const char* preferredCardName;
    int preferredCardId;
    int debugLevel;

    DeviceInitParams()
        : preferredCardName(nullptr)
        , preferredCardId(-1)
        , debugLevel(0)
    {}
};

typedef IDevice* (*RendererInitFunc)(const DeviceInitParams*);
typedef void (*RendererReleaseFunc)();

} // namespace Renderer
} // namespace NFE
