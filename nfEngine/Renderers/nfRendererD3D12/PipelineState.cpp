/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's pipeline state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"
#include "ResourceBinding.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

PipelineState::PipelineState()
    : mBindingLayout(nullptr)
{
}

bool PipelineState::Init(const PipelineStateDesc& desc)
{
    // validate

    if (desc.numRenderTargets > MAX_RENDER_TARGETS)
    {
        LOG_ERROR("Too many render targets: %u (max is %u)",
                  desc.numRenderTargets, MAX_RENDER_TARGETS);
        return false;
    }

    ResourceBindingLayout* resBindingLayout = dynamic_cast<ResourceBindingLayout*>(desc.resBindingLayout);
    if (!resBindingLayout)
    {
        LOG_ERROR("Invalid resource binding layout");
        return false;
    }


    // prepare D3D12 rasterizer state
    D3D12_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.CullMode = TranslateCullMode(desc.raterizerState.cullMode);
    rasterizerDesc.FillMode = TranslateFillMode(desc.raterizerState.fillMode);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    rasterizerDesc.DepthBias = FALSE;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


    // prepare D3D12 depth stencil state

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
    depthStencilDesc.DepthEnable = desc.depthState.depthTestEnable;
    depthStencilDesc.DepthWriteMask = desc.depthState.depthWriteEnable ?
        D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = TranslateComparisonFunc(desc.depthState.depthCompareFunc);
    const D3D12_DEPTH_STENCILOP_DESC stencilOpDesc =
    {
        TranslateStencilOp(desc.depthState.stencilOpFail),
        TranslateStencilOp(desc.depthState.stencilOpDepthFail),
        TranslateStencilOp(desc.depthState.stencilOpPass),
        TranslateComparisonFunc(desc.depthState.stencilFunc),
    };
    depthStencilDesc.FrontFace = depthStencilDesc.BackFace = stencilOpDesc;
    depthStencilDesc.StencilEnable = desc.depthState.stencilEnable;
    depthStencilDesc.StencilReadMask = desc.depthState.stencilMask;
    depthStencilDesc.StencilWriteMask = desc.depthState.stencilMask;


    // prepare D3D12 blend state

    D3D12_BLEND_DESC blendDesc;

    blendDesc.AlphaToCoverageEnable = desc.blendState.alphaToCoverage;
    blendDesc.IndependentBlendEnable = desc.blendState.independent;
    int iterations = desc.blendState.independent ? MAX_RENDER_TARGETS : 1;
    for (int i = 0; i < iterations; ++i)
    {
        const RenderTargetBlendStateDesc& rtDesc = desc.blendState.rtDescs[i];
        blendDesc.RenderTarget[i].BlendEnable = rtDesc.enable;
        blendDesc.RenderTarget[i].SrcBlend = TranslateBlendFunc(rtDesc.srcColorFunc);
        blendDesc.RenderTarget[i].DestBlend = TranslateBlendFunc(rtDesc.destColorFunc);
        blendDesc.RenderTarget[i].SrcBlendAlpha = TranslateBlendFunc(rtDesc.srcAlphaFunc);
        blendDesc.RenderTarget[i].DestBlendAlpha = TranslateBlendFunc(rtDesc.destAlphaFunc);
        blendDesc.RenderTarget[i].BlendOp = TranslateBlendOp(rtDesc.colorOperator);
        blendDesc.RenderTarget[i].BlendOpAlpha = TranslateBlendOp(rtDesc.alphaOperator);
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    // workaround for D3D runtime bug
    for (int i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_SET;
        blendDesc.RenderTarget[i].LogicOpEnable = FALSE;
    }

    // prepare D3D12 input layout

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
    VertexLayout* vertexLayout = dynamic_cast<VertexLayout*>(desc.vertexLayout);
    inputLayoutDesc.NumElements = static_cast<UINT>(vertexLayout->mElements.size());
    inputLayoutDesc.pInputElementDescs = vertexLayout->mElements.data();



    Shader* mVS = dynamic_cast<Shader*>(desc.vertexShader);
    Shader* mHS = dynamic_cast<Shader*>(desc.hullShader);
    Shader* mDS = dynamic_cast<Shader*>(desc.domainShader);
    Shader* mGS = dynamic_cast<Shader*>(desc.geometryShader);
    Shader* mPS = dynamic_cast<Shader*>(desc.pixelShader);

    D3D12_SHADER_BYTECODE nullBytecode;
    nullBytecode.BytecodeLength = 0;
    nullBytecode.pShaderBytecode = nullptr;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psd;
    ZeroMemory(&psd, sizeof(psd));

    psd.pRootSignature = resBindingLayout->GetD3DRootSignature();
    psd.VS = mVS ? mVS->GetD3D12Bytecode() : nullBytecode;
    psd.HS = mHS ? mHS->GetD3D12Bytecode() : nullBytecode;
    psd.DS = mDS ? mDS->GetD3D12Bytecode() : nullBytecode;
    psd.GS = mGS ? mGS->GetD3D12Bytecode() : nullBytecode;
    psd.PS = mPS ? mPS->GetD3D12Bytecode() : nullBytecode;
    psd.StreamOutput.NumEntries = 0;
    psd.StreamOutput.NumStrides = 0;
    psd.BlendState = blendDesc;
    psd.RasterizerState = rasterizerDesc;
    psd.DepthStencilState = depthStencilDesc;
    psd.InputLayout = inputLayoutDesc;
    psd.SampleMask = UINT_MAX;
    psd.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psd.PrimitiveTopologyType = TranslatePrimitiveTopologyType(desc.primitiveType);

    psd.NumRenderTargets = desc.numRenderTargets;
    for (uint32 i = 0; i < desc.numRenderTargets; ++i)
    {
        psd.RTVFormats[i] = TranslateElementFormat(desc.rtFormats[i]);
        if (psd.RTVFormats[i] == DXGI_FORMAT_UNKNOWN)
        {
            LOG_ERROR("Invalid render target framt for i = %u", i);
            return false;
        }
    }

    for (uint32 i = desc.numRenderTargets; i < MAX_RENDER_TARGETS; ++i)
        psd.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
    psd.DSVFormat = TranslateDepthFormat(desc.depthFormat);

    psd.SampleDesc.Count = 1;
    psd.SampleDesc.Quality = 0;
    psd.NodeMask = 1;
    psd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    psd.CachedPSO.CachedBlobSizeInBytes = 0;
    psd.CachedPSO.pCachedBlob = nullptr;


    HRESULT hr;
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateGraphicsPipelineState(&psd,
                                                                          IID_PPV_ARGS(&mPipelineState)));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create pipeline state object");
        return false;
    }

    mBindingLayout = resBindingLayout;
    mPrimitiveTopology = TranslatePrimitiveType(desc.primitiveType, desc.numControlPoints);

    return true;
}

} // namespace Renderer
} // namespace NFE
