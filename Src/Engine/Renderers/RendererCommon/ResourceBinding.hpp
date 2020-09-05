/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Shader.hpp"
#include "Types.hpp"
#include "Texture.hpp"
#include "Buffer.hpp"

namespace NFE {
namespace Renderer {

// max supported number of binding sets per layout
#define NFE_RENDERER_MAX_BINDING_SETS 16

// max supported number of resources in one set
// TODO this is quite low right now, which makes things more convenient in D3D12 implementation, bump it later
#define NFE_RENDERER_MAX_RESOURCES_IN_SET 64

struct ResourceBindingDesc
{
    ShaderResourceType resourceType;
    uint32 slot; //< shader's resource slot
    // TODO support for ranges of bindings

    SamplerPtr staticSampler; //< optional static sampler (for textures only)

    ResourceBindingDesc()
        : resourceType(ShaderResourceType::Unknown)
        , slot(0)
        , staticSampler(nullptr)
    { }

    ResourceBindingDesc(ShaderResourceType resourceType, uint32 slot, SamplerPtr staticSampler = nullptr)
        : resourceType(resourceType)
        , slot(slot)
        , staticSampler(staticSampler)
    { }
};

struct ResourceBindingSetDesc
{
    const ResourceBindingDesc* resourceBindings;
    uint32 numBindings;
    ShaderType shaderVisibility;

    ResourceBindingSetDesc()
        : resourceBindings(nullptr)
        , numBindings(0)
        , shaderVisibility(ShaderType::Unknown)
    { }

    ResourceBindingSetDesc(const ResourceBindingDesc* resourceBindings, uint32 numBindings, ShaderType shaderVisibility)
        : resourceBindings(resourceBindings)
        , numBindings(numBindings)
        , shaderVisibility(shaderVisibility)
    { }
};

/**
 * Shader resource binding set.
 * Represents a list of shader resource bindings.
 */
class IResourceBindingSet
{
public:
    virtual ~IResourceBindingSet() { }
    virtual bool Init(const ResourceBindingSetDesc& desc) = 0;
};


//////////////////////////////////////////////////////////////////////////


/**
 * Buffers that will be updated frequently (e.g. constant buffers) must be
 * bound via this structure.
 */
struct VolatileCBufferBinding
{
    ShaderType shaderVisibility;
    ShaderResourceType resourceType;
    uint32 slot; //< shader's resource slot
    uint32 size;

    VolatileCBufferBinding(ShaderType shaderVisibility, ShaderResourceType resourceType, uint32 slot, uint32 size)
        : shaderVisibility(shaderVisibility)
        , resourceType(resourceType)
        , slot(slot)
        , size(size)
    {
    }
};

struct ResourceBindingLayoutDesc
{
    const ResourceBindingSetPtr* bindingSets;
    uint32 numBindingSets;

    const VolatileCBufferBinding* volatileCBuffers;
    uint32 numVolatileCBuffers;

    const char* debugName;

    // TODO:
    // fast constants (Vulkan's Push Constants / D3D12 Root Constants)
    // immutable / static samplers

    ResourceBindingLayoutDesc()
        : bindingSets(nullptr)
        , numBindingSets(0)
        , volatileCBuffers(nullptr)
        , numVolatileCBuffers(0)
        , debugName(nullptr)
    { }

    ResourceBindingLayoutDesc(const ResourceBindingSetPtr* bindingSets, uint32 numBindingSets,
                              const VolatileCBufferBinding* volatileCBuffers = nullptr,
                              uint32 numVolatileCBuffers = 0, const char* debugName = nullptr)
        : bindingSets(bindingSets)
        , numBindingSets(numBindingSets)
        , volatileCBuffers(volatileCBuffers)
        , numVolatileCBuffers(numVolatileCBuffers)
        , debugName(debugName)
    { }
};

/**
 * Shader resource binding layout.
 * Contains a list of resource binding sets. Describes how resources can be bound to the pipeline.
 */
class IResourceBindingLayout
{
public:
    virtual ~IResourceBindingLayout() { }
    virtual bool Init(const ResourceBindingLayoutDesc& desc) = 0;
};


//////////////////////////////////////////////////////////////////////////


/**
 * Instance of shader resource binding set.
 * Contains an actual list of shader resource views that can be bound to the pipeline.
 * Binding instance cannot be used before it's finalized.
 */
class IResourceBindingInstance
{
public:
    virtual ~IResourceBindingInstance() { }

    virtual bool Init(const ResourceBindingSetPtr& bindingSet) = 0;

    // TODO: custom samplers
    /**
     * Create a texture view in the binding instance.
     * @param slot     Target slot within the binding set.
     * @param texture  Texture resource.
     * @return True on success.
     */
    virtual bool SetTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view = TextureView()) = 0;

    /**
     * Create a buffer view in the binding instance.
     * @param slot Target slot within the binding set.
     * @return True on success.
     */
    virtual bool SetBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view = BufferView()) = 0;

    /**
     * Create a constant buffer view in the binding instance.
     * @param slot Target slot within the binding set.
     * @return True on success.
     */
    virtual bool SetCBufferView(uint32 slot, const BufferPtr& buffer) = 0;

    /**
     * Create a writable texture view in the binding instance.
     * @param slot     Target slot within the binding set.
     * @param texture  Shader-writable texture resource.
     * @return True on success.
     */
    virtual bool SetWritableTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view = TextureView()) = 0;

    /**
     * Create a writable buffer view in the binding instance.
     * @param slot     Target slot within the binding set.
     * @param texture  Shader-writable buffer resource.
     * @return True on success.
     */
    virtual bool SetWritableBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view = BufferView()) = 0;

    /**
     * Finalize instance building. No further changes can be applied afterwards.
     */
    virtual bool Finalize() = 0;
};


} // namespace Renderer
} // namespace NFE
