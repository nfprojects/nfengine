/**
 * @file
 * @author  Witek902
 * @brief   Per-shader-stage descriptor set manager
 */

#pragma once

#include "Descriptors.hpp"

#include "../RendererCommon/Texture.hpp"
#include "../RendererCommon/Buffer.hpp"

namespace NFE {
namespace Renderer {

class DescriptorSet
{
public:
    DescriptorSet(ShaderType shaderStage);

    // reset to default state (all descriptors are dirty)
    void Reset();

    void SetConstantBuffer(uint32 slot, const BufferPtr& buffer);
    void SetTexture(uint32 slot, const TexturePtr& texture, const TextureView& view);
    void SetWritableTexture(uint32 slot, const TexturePtr& texture, const TextureView& view);
    void SetBuffer(uint32 slot, const BufferPtr& buffer, const BufferView& view);
    void SetWritableBuffer(uint32 slot, const BufferPtr& buffer, const BufferView& view);
    void SetSampler(uint32 slot, const SamplerPtr& sampler);

    // check what bindings are dirty and update descriptor tables if needed
    void ApplyChanges(
        ID3D12GraphicsCommandList* commandList,
        ReferencedDescriptorsRanges& outReferencedCbvSrvUavDescriptorRanges,
        ReferencedDescriptorsRanges& outReferencedSamplerDescriptorRanges);

    bool SupportsUAV() const
    {
        return mShaderStage == ShaderType::Pixel || mShaderStage == ShaderType::Compute;
    }

private:

    ShaderType mShaderStage;

    // TODO use bitfields
    uint8 mDirtyCBVs;
    uint32 mDirtySRVs;
    uint16 mDirtyUAVs;
    uint16 mDirtySamplers;

    DescriptorID mDescriptorsCBV[NFE_MAX_CBUFFER_SLOTS];
    DescriptorID mDescriptorsSRV[NFE_MAX_SHADER_RESOURCE_SLOTS];
    DescriptorID mDescriptorsUAV[NFE_MAX_WRITABLE_SHADER_RESOURCE_SLOTS];
    DescriptorID mDescriptorsSampler[NFE_MAX_SAMPLER_SLOTS];

    void UpdateDirtyCBVs(ID3D12GraphicsCommandList* commandList, ReferencedDescriptorsRanges& outReferencedDescriptorRanges);
    void UpdateDirtySRVs(ID3D12GraphicsCommandList* commandList, ReferencedDescriptorsRanges& outReferencedDescriptorRanges);
    void UpdateDirtyUAVs(ID3D12GraphicsCommandList* commandList, ReferencedDescriptorsRanges& outReferencedDescriptorRanges);
    void UpdateDirtySamplers(ID3D12GraphicsCommandList* commandList, ReferencedDescriptorsRanges& outReferencedDescriptorRanges);
};

} // namespace Renderer
} // namespace NFE
