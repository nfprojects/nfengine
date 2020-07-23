/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 * @details The interface is communication layer between rendering API (such as Direct3D or Vulkan)
 *          and high-level engine's renderer. The API provides basic abstract classes:
 *          vertex buffers, index buffers, shader constant buffers, renderer states, textures,
 *          rendertargets, etc.
 */

#pragma once

#include "CommandRecorder.hpp"

#include "../../Common/Containers/DynArray.hpp"
#include "../../Common/Containers/String.hpp"


#define RENDERER_INIT_FUNC "Init"
#define RENDERER_RELEASE_FUNC "Release"

namespace NFE {
namespace Renderer {

/**
 * Rendering device information.
 */
struct DeviceInfo
{
    Common::String description;  //< GPU name
    Common::String misc;         //< miscellaneous GPU info
    Common::DynArray<Common::String> features; //< list of supported features (depends on low-level API)
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
    virtual bool IsBackbufferFormatSupported(Format format) = 0;

    /**
     * DeviceResourcesCreation Resources creation functions
     * @{
     */

    virtual VertexLayoutPtr CreateVertexLayout(const VertexLayoutDesc& desc) = 0;
    virtual BufferPtr CreateBuffer(const BufferDesc& desc) = 0;
    virtual TexturePtr CreateTexture(const TextureDesc& desc) = 0;
    virtual BackbufferPtr CreateBackbuffer(const BackbufferDesc& desc) = 0;
    virtual RenderTargetPtr CreateRenderTarget(const RenderTargetDesc& desc) = 0;
    virtual PipelineStatePtr CreatePipelineState(const PipelineStateDesc& desc) = 0;
    virtual ComputePipelineStatePtr CreateComputePipelineState(const ComputePipelineStateDesc& desc) = 0;
    virtual SamplerPtr CreateSampler(const SamplerDesc& desc) = 0;
    virtual ShaderPtr CreateShader(const ShaderDesc& desc) = 0;
    virtual ResourceBindingSetPtr CreateResourceBindingSet(const ResourceBindingSetDesc& desc) = 0;
    virtual ResourceBindingLayoutPtr CreateResourceBindingLayout(const ResourceBindingLayoutDesc& desc) = 0;
    virtual ResourceBindingInstancePtr CreateResourceBindingInstance(const ResourceBindingSetPtr& set) = 0;

    /**@}*/

    /**
     * Create a command recorder.
     */
    virtual CommandRecorderPtr CreateCommandRecorder() = 0;

    /**
     * Waits until all operations sent to the command queue has been completed.
     * @return Fence object.
     */
    virtual FencePtr WaitForGPU() = 0;

    /**
     * Execute a command lists.
     * @param commandLists Array of command list to be executed.
     * @return True on success.
     */
    virtual bool Execute(const Common::ArrayView<ICommandList*> commandLists) = 0;

    NFE_FORCE_INLINE bool Execute(const CommandListPtr& commandList)
    {
        ICommandList* commandListPtr = commandList.Get();
        return Execute(Common::ArrayView<ICommandList*>(&commandListPtr, 1u));
    }

    /**
     * Inform Renderer about finished frame.
     *
     * @return True on success.
     *
     * @remarks This function will invalidate all generated Command Lists (even non-executed ones).
     */
    virtual bool FinishFrame() = 0;

    /**
     * Read data from a GPU buffer to the CPU memory.
     * @param      buffer Source buffer.
     * @param      offset Offset in the GPU buffer (in bytes).
     * @param      size   Number of bytes to read.
     * @param[out] data   Pointer to target CPU buffer.
     * @return true on success.
     */
    virtual bool DownloadBuffer(const BufferPtr& buffer, size_t offset, size_t size, void* data) = 0;

    /**
     * Read texture content to a CPU buffer.
     * @param      tex  Texture to read.
     * @param[out] data Target CPU buffer.
     */
    virtual bool DownloadTexture(const TexturePtr& tex, void* data, uint32 mipmap = 0, uint32 layer = 0) = 0;
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
