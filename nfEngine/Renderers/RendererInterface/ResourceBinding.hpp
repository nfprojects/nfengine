/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Shader.hpp"
#include "Types.hpp"

namespace NFE {
namespace Renderer {

struct ResourceBindingDesc
{
    ShaderResourceType resourceType;
    int baseSlot; //< base shader slot
    int numResources; //< number of resources (slots)

    ResourceBindingDesc()
        : resourceType(ShaderResourceType::CBuffer)
        , baseSlot(0)
        , numResources(0)
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

// ================================================================================================

/**
 * Instance of shader resource bining set.
 * Contains a list of shader resource views.
 */
class IResourceBindingInstance
{
public:
    virtual ~IResourceBindingInstance() { }
    virtual bool Init(IResourceBindingSet* bindingSet) = 0;
};

// ================================================================================================

struct ResourceBindingLayoutDesc
{
    IResourceBindingSet* bindingSets;
    size_t numBindingSets;

    // TODO constants (Vulkan's Push Constants / D3D12 Root Constants)

    ResourceBindingLayoutDesc()
        : bindingSets(nullptr)
        , numBindingSets(0)
    { }
};

/**
 * Shader resource binding layout.
 *
 * Contains a list of resource bining sets.
 */
class IResourceBindingLayout
{
public:
    virtual ~IResourceBindingLayout() { }
    virtual bool Init(const ResourceBindingLayoutDesc& desc) = 0;
};

} // namespace Renderer
} // namespace NFE
