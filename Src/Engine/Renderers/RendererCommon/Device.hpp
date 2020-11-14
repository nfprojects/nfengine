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

#include "CommandQueue.hpp"

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

struct TexturePlacementInfo
{
    uint32 alignment;
    size_t rowPitch;
    size_t totalSize;
};

using ResourceDownloadCallback = std::function<void(const void*, size_t dataSize, size_t rowPitch)>;

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
     * Get texture data placement requirements when copying between texture and buffer resources.
     */
    virtual bool CalculateTexturePlacementInfo(Format format, uint32 width, uint32 height, uint32 depth, TexturePlacementInfo& outInfo) const = 0;

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
     * Create a command queue.
     */
    virtual CommandQueuePtr CreateCommandQueue(CommandQueueType type, const char* debugName = nullptr) = 0;

    /**
     * Inform Renderer about finished frame.
     *
     * @return True on success.
     *
     * @remarks This function will invalidate all generated Command Lists (even non-executed ones).
     */
    virtual bool FinishFrame() = 0;
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
