/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's shader resources bindings
 */

#pragma once

#include "../RendererInterface/ResourceBinding.hpp"
#include "Common.hpp"
#include "Texture.hpp"

namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
public:
    bool Init(const ResourceBindingSetDesc& desc) override;
};

class ResourceBindingInstance : public IResourceBindingInstance
{
public:
    bool Init(IResourceBindingSet* bindingSet) override;
};

class ResourceBindingLayout : public IResourceBindingLayout
{
    friend class PipelineState;

    D3DPtr<ID3D12RootSignature> mRootSignature;

public:
    bool Init(const ResourceBindingLayoutDesc& desc) override;
};

} // namespace Renderer
} // namespace NFE
