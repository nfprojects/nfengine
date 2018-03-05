/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 renderer's shader resource binding
 */

#pragma once

#include "../RendererInterface/ResourceBinding.hpp"
#include "Common.hpp"
#include "../../nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
    friend class ResourceBindingInstance;
    friend class CommandRecorder;

    Common::DynArray<ResourceBindingDesc> mBindings;
    ShaderType mShaderVisibility;

public:
    bool IsBindingOverlapping(const ResourceBindingDesc& bindingDesc) const;
    bool IsBindingSetOverlapping(const ResourceBindingSet* set) const;
    bool Init(const ResourceBindingSetDesc& desc);
};

using InternalResourceBindingSetPtr = Common::SharedPtr<ResourceBindingSet>;


class ResourceBindingLayout : public IResourceBindingLayout
{
    friend class CommandRecorder;

    Common::DynArray<InternalResourceBindingSetPtr> mBindingSets;
    Common::DynArray<VolatileCBufferBinding> mVolatileCBuffers;

public:
    bool Init(const ResourceBindingLayoutDesc& desc);
};

class ResourceBindingInstance : public IResourceBindingInstance
{
    friend class CommandRecorder;

    InternalResourceBindingSetPtr mBindingSet;
    Common::DynArray<D3DPtr<ID3D11View>> mViews;
    Common::DynArray<ID3D11Buffer*> mCBuffers;

public:
    bool Init(const ResourceBindingSetPtr& bindingSet);
    bool WriteTextureView(uint32 slot, const TexturePtr& texture) override;
    bool WriteCBufferView(uint32 slot, const BufferPtr& buffer) override;
    bool WriteWritableTextureView(uint32 slot, const TexturePtr& texture) override;
};

} // namespace Renderer
} // namespace NFE
