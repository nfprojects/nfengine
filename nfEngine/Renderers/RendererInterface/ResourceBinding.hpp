/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Shader.hpp"
#include "Types.hpp"
#include "Texture.hpp"

namespace NFE {
namespace Renderer {

struct ResourceBindingDesc
{
    ShaderResourceType resourceType;
    unsigned int slot; //< shader's resource slot
    // TODO support for ranges of bindings

    SamplerPtr staticSampler; //< optional static sampler (for textures only)

    ResourceBindingDesc()
        : resourceType(ShaderResourceType::Unknown)
        , slot(0)
        , staticSampler(nullptr)
    { }

    ResourceBindingDesc(ShaderResourceType resourceType, unsigned int slot,
                        SamplerPtr staticSampler = nullptr)
        : resourceType(resourceType)
        , slot(slot)
        , staticSampler(staticSampler)
    { }
};

struct ResourceBindingSetDesc
{
    const ResourceBindingDesc* resourceBindings;
    size_t numBindings;
    ShaderType shaderVisibility;

    ResourceBindingSetDesc()
        : resourceBindings(nullptr)
        , numBindings(0)
        , shaderVisibility(ShaderType::Unknown)
    { }

    ResourceBindingSetDesc(const ResourceBindingDesc* resourceBindings,
                           size_t numBindings,
                           ShaderType shaderVisibility)
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
    unsigned int slot; //< shader's resource slot
    unsigned int size;

    VolatileCBufferBinding(ShaderType shaderVisibility,
                           ShaderResourceType resourceType,
                           unsigned int slot,
                           unsigned int size)
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
    size_t numBindingSets;

    const VolatileCBufferBinding* volatileCBuffers;
    size_t numVolatileCBuffers;

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

    ResourceBindingLayoutDesc(const ResourceBindingSetPtr* bindingSets, size_t numBindingSets,
                              const VolatileCBufferBinding* volatileCBuffers = nullptr,
                              size_t numVolatileCBuffers = 0, const char* debugName = nullptr)
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
    virtual bool WriteTextureView(size_t slot, const TexturePtr& texture) = 0;

    /**
     * Create a constant buffer view in the binding instance.
     * @param slot Target slot within the binding set.
     * @return True on success.
     */
    virtual bool WriteCBufferView(size_t slot, const BufferPtr& buffer) = 0;

    /**
     * Create a writable texture view in the binding instance.
     * @param slot     Target slot within the binding set.
     * @param texture  Shader-writable texture resource.
     * @return True on success.
     */
    virtual bool WriteWritableTextureView(size_t slot, const TexturePtr& texture) = 0;
};


} // namespace Renderer
} // namespace NFE
