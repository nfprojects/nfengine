/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's shader resources bindings
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "RendererD3D12.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    UNUSED(desc);
    return false;
}

bool ResourceBindingInstance::Init(IResourceBindingSet* bindingSet)
{
    UNUSED(bindingSet);
    return false;
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    HRESULT hr;

    // TODO: build parameters list
    UNUSED(desc);

    D3D12_ROOT_SIGNATURE_DESC rsd;
    rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rsd.NumParameters = 0;
    rsd.NumStaticSamplers = 0;

    D3DPtr<ID3D10Blob> rootSignature;
    D3DPtr<ID3D10Blob> errorsBuffer;
    hr = D3D_CALL_CHECK(D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1,
                                                    &rootSignature, &errorsBuffer));
    if (FAILED(hr))
        return false;

    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateRootSignature(0,
                                                                  rootSignature->GetBufferPointer(),
                                                                  rootSignature->GetBufferSize(),
                                                                  IID_PPV_ARGS(&mRootSignature)));
    if (FAILED(hr))
        return false;

    return true;
}

} // namespace Renderer
} // namespace NFE
