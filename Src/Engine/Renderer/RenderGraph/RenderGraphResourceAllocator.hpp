#pragma once

#include "../RendererApi.hpp"
#include "../../Renderers/RendererCommon/Texture.hpp"
#include "../../Renderers/RendererCommon/Buffer.hpp"


namespace NFE {
namespace Renderer {

using RenderGraphResourceName = const char*;

class RenderGraphResourceRef
{
    friend class RenderGraphResourceAllocator;

public:
    ~RenderGraphResourceRef() = default;

    NFE_FORCE_INLINE operator const BufferPtr& () const { return mBuffer; }
    NFE_FORCE_INLINE operator const TexturePtr& () const { return mTexture; }
    NFE_FORCE_INLINE RenderGraphResourceName GetName() const { return mName; }

private:
    RenderGraphResourceRef(RenderGraphResourceName name);
    RenderGraphResourceRef(RenderGraphResourceName name, const TexturePtr& texture);
    RenderGraphResourceRef(RenderGraphResourceName name, const BufferPtr& buffer);

    RenderGraphResourceName mName;
    BufferPtr mBuffer;
    TexturePtr mTexture;
};

class RenderGraphScopedResourceRef
{
    friend class RenderGraphResourceAllocator;

public:
    ~RenderGraphScopedResourceRef();

private:
    RenderGraphScopedResourceRef(const RenderGraphResourceRef& resourceRef, RenderGraphResourceAllocator& allocator);

    const RenderGraphResourceRef& mResourceRef;
    RenderGraphResourceAllocator& mAllocator;
};

/**
 * Class responsible for allocating and managing temporary render graph resources (buffers and textures).
 * The resources are referenced by name.
 * Each resource must be declared first, before any use.
 * Then, each resource lifetime is defined with Begin/EndUseResource.
 * Resource's memory which lifetime ends, can be reused with other, non-overlapping resource.
 */
class RenderGraphResourceAllocator
{
public:

    using ResourceRef = RenderGraphResourceRef;
    using ResourceName = RenderGraphResourceName;

    RenderGraphResourceAllocator();
    ~RenderGraphResourceAllocator();

    // Declare a new render graph resource.
    ResourceRef Declare(ResourceName name, const TextureDesc& desc);
    ResourceRef Declare(ResourceName name, const BufferDesc& desc);

    // Get declared resource
    ResourceRef GetResource(ResourceName name);

    // Mark beginning of resource use
    void BeginUseResource(ResourceName name);

    // Mark end of resource use
    void EndUseResource(ResourceName name);

    // Get scope helper which handles Begin/End automatically
    RenderGraphScopedResourceRef GetScopedResource(ResourceName name);

    // Prepare for recording resource allocations
    void Reset();

    // Resolve resource allocations, allocate memory, create resources
    void Resolve();

private:

    enum class ResourceType : uint8
    {
        Buffer,
        Texture
    };

    struct ResourceData
    {
        ResourceType type = ResourceType::Buffer;
        RenderGraphResourceName name = nullptr;

        // TODO can't use union, because of ShaderPtr inside
        BufferDesc bufferDesc;
        TextureDesc textureDesc;

        BufferPtr buffer;
        TexturePtr texture;

        // TODO begin/end markers must be tracked per command list
        uint32 beginUseMarker = UINT32_MAX;
        uint32 endUseMarker = 0;
    };

    // resources allocation state for the current frame
    Common::HashMap<uint64, ResourceData> mResources;

    // resources allocations from previous frame
    // without the cache, all the resources would be recreated every frame
    Common::DynArray<ResourceData> mResourcesCache;

    bool mPreparePhase;

    // TODO event counter must be tracked per command list
    uint32 mEventCounter;
};

} // namespace Renderer
} // namespace NFE
