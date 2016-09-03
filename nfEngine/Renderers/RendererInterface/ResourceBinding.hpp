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

    ISampler* staticSampler; //< optional static sampler (for textures only)

    ResourceBindingDesc()
        : resourceType(ShaderResourceType::Unknown)
        , slot(0)
        , staticSampler(nullptr)
    { }

    ResourceBindingDesc(ShaderResourceType resourceType, unsigned int slot,
                        ISampler* staticSampler = nullptr)
        : resourceType(resourceType)
        , slot(slot)
        , staticSampler(staticSampler)
    { }
};

struct ResourceBindingSetDesc
{
    ResourceBindingDesc* resourceBindings;
    size_t numBindings;
    ShaderType shaderVisibility;

    ResourceBindingSetDesc()
        : resourceBindings(nullptr)
        , numBindings(0)
        , shaderVisibility(ShaderType::Unknown)
    { }

    ResourceBindingSetDesc(ResourceBindingDesc* resourceBindings,
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

/**
 * Buffers that will be updated frequently (e.g. constant buffers) must be
 * bound via this structure.
 */
struct DynamicBufferBindingDesc
{
    ShaderType shaderVisibility;
    ShaderResourceType resourceType;
    unsigned int slot; //< shader's resource slot

    DynamicBufferBindingDesc(ShaderType shaderVisibility,
                             ShaderResourceType resourceType,
                             unsigned int slot)
        : shaderVisibility(shaderVisibility)
        , resourceType(resourceType)
        , slot(slot)
    {
    }
};

struct ResourceBindingLayoutDesc
{
    IResourceBindingSet** bindingSets;
    size_t numBindingSets;

    DynamicBufferBindingDesc* dynamicBuffers;
    size_t numDynamicBuffers;

    // TODO:
    // fast constants (Vulkan's Push Constants / D3D12 Root Constants)
    // immutable / static samplers

    ResourceBindingLayoutDesc()
        : bindingSets(nullptr)
        , numBindingSets(0)
        , dynamicBuffers(nullptr)
        , numDynamicBuffers(0)
    { }

    ResourceBindingLayoutDesc(IResourceBindingSet** bindingSets, size_t numBindingSets,
                              DynamicBufferBindingDesc* dynamicBuffers = nullptr,
                              size_t numDynamicBuffers = 0)
        : bindingSets(bindingSets)
        , numBindingSets(numBindingSets)
        , dynamicBuffers(dynamicBuffers)
        , numDynamicBuffers(numDynamicBuffers)
    { }
};

/**
 * Shader resource binding layout.
 * Contains a list of resource bining sets. Describes how resources can be bound to the pipeline.
 */
class IResourceBindingLayout
{
public:
    virtual ~IResourceBindingLayout() { }
    virtual bool Init(const ResourceBindingLayoutDesc& desc) = 0;
};


/**
 * Instance of shader resource bining set.
 * Contains an actual list of shader resource views that can be bound to the pipeline.
 */
class IResourceBindingInstance
{
public:
    virtual ~IResourceBindingInstance() { }

    virtual bool Init(IResourceBindingSet* bindingSet) = 0;

    // TODO: custom samplers
    /**
     * Create a texture view in the binding instance.
     * @param slot     Target slot within the binding set.
     * @param texture  Texture resource.
     * @return True on success.
     */
    virtual bool WriteTextureView(size_t slot, ITexture* texture) = 0;

    /**
     * Create a constant buffer view in the binding instance.
     * @param slot Target slot within the binding set.
     * @return True on success.
     */
    virtual bool WriteCBufferView(size_t slot, IBuffer* buffer) = 0;

    // TODO: unorderer access view, buffers and others
};

} // namespace Renderer
} // namespace NFE
