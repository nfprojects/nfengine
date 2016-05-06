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

bool PipelineState::Init(const PipelineStateDesc& desc)
{
    // prepare D3D12 rasterizer state

    switch (desc.raterizerState.cullMode)
    {
    case CullMode::CW:
        mRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        break;
    case CullMode::CCW:
        mRasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
        break;
    case CullMode::Disabled:
        mRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        break;
    default:
        return false;
    };

    switch (desc.raterizerState.fillMode)
    {
    case FillMode::Solid:
        mRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        break;
    case FillMode::Wireframe:
        mRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
        break;
    default:
        return false;
    };

    mRasterizerDesc.FrontCounterClockwise = TRUE;
    mRasterizerDesc.DepthBias = FALSE;
    mRasterizerDesc.DepthBiasClamp = 0.0f;
    mRasterizerDesc.SlopeScaledDepthBias = 0.0f;
    mRasterizerDesc.DepthClipEnable = TRUE;
    mRasterizerDesc.MultisampleEnable = FALSE;
    mRasterizerDesc.AntialiasedLineEnable = FALSE;
    mRasterizerDesc.ForcedSampleCount = 0;
    mRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // prepare D3D12 depth stencil state

    mDepthStencilDesc.DepthEnable = desc.depthState.depthTestEnable;
    mDepthStencilDesc.DepthWriteMask = desc.depthState.depthWriteEnable ?
        D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    mDepthStencilDesc.DepthFunc = TranslateComparisonFunc(desc.depthState.depthCompareFunc);

    // TODO: stencil buffer support
    const D3D12_DEPTH_STENCILOP_DESC stencilOpDesc =
    {
        TranslateStencilOp(desc.depthState.stencilOpFail),
        TranslateStencilOp(desc.depthState.stencilOpDepthFail),
        TranslateStencilOp(desc.depthState.stencilOpPass),
        TranslateComparisonFunc(desc.depthState.stencilFunc),
    };
    mDepthStencilDesc.FrontFace = mDepthStencilDesc.BackFace = stencilOpDesc;
    mDepthStencilDesc.StencilEnable = desc.depthState.stencilEnable;
    mDepthStencilDesc.StencilReadMask = desc.depthState.stencilMask;
    mDepthStencilDesc.StencilWriteMask = desc.depthState.stencilMask;


    // prepare D3D12 blend state

    mBlendDesc.AlphaToCoverageEnable = desc.blendState.alphaToCoverage;
    mBlendDesc.IndependentBlendEnable = desc.blendState.independent;

    int iterations = desc.blendState.independent ? MAX_RENDER_TARGETS : 1;
    for (int i = 0; i < iterations; ++i)
    {
        const RenderTargetBlendStateDesc& rtDesc = desc.blendState.rtDescs[i];
        mBlendDesc.RenderTarget[i].BlendEnable = rtDesc.enable;
        mBlendDesc.RenderTarget[i].SrcBlend = TranslateBlendFunc(rtDesc.srcColorFunc);
        mBlendDesc.RenderTarget[i].DestBlend = TranslateBlendFunc(rtDesc.destColorFunc);
        mBlendDesc.RenderTarget[i].SrcBlendAlpha = TranslateBlendFunc(rtDesc.srcAlphaFunc);
        mBlendDesc.RenderTarget[i].DestBlendAlpha = TranslateBlendFunc(rtDesc.destAlphaFunc);
        mBlendDesc.RenderTarget[i].BlendOp = TranslateBlendOp(rtDesc.colorOperator);
        mBlendDesc.RenderTarget[i].BlendOpAlpha = TranslateBlendOp(rtDesc.alphaOperator);
        mBlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        mBlendDesc.RenderTarget[i].LogicOpEnable = FALSE; // TODO add support
    }


    // prepare D3D12 input layout

    VertexLayout* vertexLayout = dynamic_cast<VertexLayout*>(desc.vertexLayout);
    mInputLayoutDesc.NumElements = static_cast<UINT>(vertexLayout->mElements.size());
    mInputLayoutDesc.pInputElementDescs = vertexLayout->mElements.data();

    ResourceBindingLayout* resBindingLayout = dynamic_cast<ResourceBindingLayout*>(desc.resBindingLayout);
    if (!resBindingLayout)
    {
        LOG_ERROR("Invalid resource binding layout");
        return false;
    }
    mBindingLayout = resBindingLayout;

    return true;
}

bool FullPipelineState::Init(const FullPipelineStateParts& parts)
{
    PipelineState* pipelineState = dynamic_cast<PipelineState*>(std::get<0>(parts));
    ShaderProgram* shaderProgram = dynamic_cast<ShaderProgram*>(std::get<1>(parts));
    const ShaderProgramDesc& desc = shaderProgram->GetDesc();

    Shader* mVS = dynamic_cast<Shader*>(desc.vertexShader);
    Shader* mHS = dynamic_cast<Shader*>(desc.hullShader);
    Shader* mDS = dynamic_cast<Shader*>(desc.domainShader);
    Shader* mGS = dynamic_cast<Shader*>(desc.geometryShader);
    Shader* mPS = dynamic_cast<Shader*>(desc.pixelShader);

    D3D12_SHADER_BYTECODE nullBytecode;
    nullBytecode.BytecodeLength = 0;
    nullBytecode.pShaderBytecode = nullptr;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psd;
    psd.pRootSignature = pipelineState->mBindingLayout->GetD3DRootSignature();
    psd.VS = mVS ? mVS->GetD3D12Bytecode() : nullBytecode;
    psd.HS = mHS ? mHS->GetD3D12Bytecode() : nullBytecode;
    psd.DS = mDS ? mDS->GetD3D12Bytecode() : nullBytecode;
    psd.GS = mGS ? mGS->GetD3D12Bytecode() : nullBytecode;
    psd.PS = mPS ? mPS->GetD3D12Bytecode() : nullBytecode;
    psd.StreamOutput.NumEntries = 0;
    psd.StreamOutput.NumStrides = 0;
    psd.BlendState = pipelineState->mBlendDesc;
    psd.RasterizerState = pipelineState->mRasterizerDesc;
    psd.DepthStencilState = pipelineState->mDepthStencilDesc;
    psd.InputLayout = pipelineState->mInputLayoutDesc;
    psd.SampleMask = UINT_MAX;
    psd.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // TODO
    psd.NumRenderTargets = 1; // TODO
    psd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO
    for (int i = 1; i < 8; ++i)
        psd.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
    psd.DSVFormat = DXGI_FORMAT_D32_FLOAT; // TODO
    psd.SampleDesc.Count = 1;
    psd.SampleDesc.Quality = 0;
    psd.NodeMask = 1;
    psd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    psd.CachedPSO.CachedBlobSizeInBytes = 0;
    psd.CachedPSO.pCachedBlob = nullptr;

    HRESULT hr;
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateGraphicsPipelineState(&psd,
                                                                          IID_PPV_ARGS(&mPipelineState)));
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE
