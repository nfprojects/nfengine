#include "PCH.hpp"
#include "PostProcessRenderer.hpp"
#include "Renderer.hpp"

namespace NFE {
namespace Render {

using namespace Math;

#define AVERAGING_BUFFER_SIZE (2048)
#define BLUR_WEIGHTS_TABLE_SIZE (256)

struct ToneMappingCBuffer
{
    Float4 bufferInvRes;
    Float4 outResolution;
    Float4 properties;
    Float4 noiseSeed;
};

struct FXAACBuffer
{
    Float4 bufferInvRes;
    Float4 outResolution;
    Float4 properties;
    Float4 noiseSeed;
};


struct BlurCBuffer
{
    uint32 inputRes[4];
    Float4 params;
};

struct MotionBlurCBuffer
{
    Float4 bufferInvRes;
    Float4 outResolution;
    Float4 factor;
    UINT seed[4];
};


/*
struct XAntialiasCBuffer
{
    Float4 bufferInvRes;
};
*/

PostProcessRendererD3D11::PostProcessRendererD3D11()
{
    mToneMappingCBuffer = 0;
    mFXAACBuffer = 0;
    mDownsamplingCBuffer = 0;
    mMotionBlurCBuffer = 0;
    mBlurCBuffer = 0;

    mAveragingBufferUAV = 0;
    mTempAveragingBuffer = 0;
    mAveragingBuffer = 0;

    mBorderSamplerState = 0;
    mClampSamplerState = 0;
    mMotionBlurSamplerState = 0;
}

PostProcessRendererD3D11::~PostProcessRendererD3D11()
{
    Release();
}

Result PostProcessRendererD3D11::Init(RendererD3D11* pRenderer)
{
    mAveragingCS.Init(pRenderer, ShaderType::Compute, "AveragingCS");

    mAntialiasPS.Init(pRenderer, ShaderType::Pixel, "AntialiasPS");

    mToneMappingPS.AddMacro("BLOOM", 1, 0);
    mToneMappingPS.AddMacro("GAMMA_CORRECTION", 1, 1);
    mToneMappingPS.AddMacro("NOISE", 1, 2);
    mToneMappingPS.Init(pRenderer, ShaderType::Pixel, "ToneMappingPS");

    mFXAAPS.AddMacro("ANTIALIASING", 1, 0);
    mFXAAPS.AddMacro("LUMA_OPT", 1, 1);
    mFXAAPS.AddMacro("NOISE", 1, 2);
    mFXAAPS.Init(pRenderer, ShaderType::Pixel, "FXAAPS");

    mDownsamplingPS.Init(pRenderer, ShaderType::Pixel, "DownsamplingPS");

    mMotionBlurPS.Init(pRenderer, ShaderType::Pixel, "MotionBlurPS");

    mBlurPS.AddMacro("MODE", 1, 0);
    mBlurPS.Init(pRenderer, ShaderType::Pixel, "BlurPS");

    mBlurCS.AddMacro("MODE", 1, 0);
    mBlurCS.Init(pRenderer, ShaderType::Compute, "BlurCS");


    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;
    bd.ByteWidth = sizeof(ToneMappingCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mToneMappingCBuffer));

    bd.MiscFlags = 0;
    bd.ByteWidth = sizeof(FXAACBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mFXAACBuffer));

    bd.ByteWidth = sizeof(Float4);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mDownsamplingCBuffer));

    bd.ByteWidth = sizeof(MotionBlurCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mMotionBlurCBuffer));

    bd.ByteWidth = sizeof(BlurCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mBlurCBuffer));

    bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bd.ByteWidth = sizeof(float) * AVERAGING_BUFFER_SIZE;
    bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = sizeof(float);
    bd.Usage = D3D11_USAGE_DEFAULT;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mTempAveragingBuffer));
    D3D_CHECK(pRenderer->D3DDevice->CreateUnorderedAccessView(mTempAveragingBuffer, 0,
              &mAveragingBufferUAV));


    bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bd.ByteWidth = sizeof(float) * AVERAGING_BUFFER_SIZE;
    bd.BindFlags = 0;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    bd.StructureByteStride = sizeof(float);
    bd.Usage = D3D11_USAGE_STAGING;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mAveragingBuffer));


    RendererD3D11SamplerDesc samplerDesc(D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
                                         D3D11_TEXTURE_ADDRESS_BORDER);
    D3D_CHECK(pRenderer->D3DDevice->CreateSamplerState(&samplerDesc, &mBorderSamplerState));

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    D3D_CHECK(pRenderer->D3DDevice->CreateSamplerState(&samplerDesc, &mClampSamplerState));

    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MaxAnisotropy = 16;
    D3D_CHECK(pRenderer->D3DDevice->CreateSamplerState(&samplerDesc, &mMotionBlurSamplerState));

    return Result::OK;
}

void PostProcessRendererD3D11::Release()
{
    D3D_SAFE_RELEASE(mAveragingBufferUAV);
    D3D_SAFE_RELEASE(mTempAveragingBuffer);
    D3D_SAFE_RELEASE(mAveragingBuffer);

    D3D_SAFE_RELEASE(mMotionBlurCBuffer);
    D3D_SAFE_RELEASE(mFXAACBuffer);
    D3D_SAFE_RELEASE(mToneMappingCBuffer);
    D3D_SAFE_RELEASE(mDownsamplingCBuffer);

    D3D_SAFE_RELEASE(mBlurCBuffer);

    D3D_SAFE_RELEASE(mClampSamplerState);
    D3D_SAFE_RELEASE(mBorderSamplerState);
    D3D_SAFE_RELEASE(mMotionBlurSamplerState);

    mAntialiasPS.Release();
    mAveragingCS.Release();
    mFXAAPS.Release();
    mToneMappingPS.Release();
    mDownsamplingPS.Release();
    mMotionBlurPS.Release();
    mBlurPS.Release();
    mBlurCS.Release();
}

void PostProcessRendererD3D11::Enter(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();


    //set up Input Assembler
    pCtx->D3DContext->IASetInputLayout(pRenderer->fullScreenQuadIL);
    pCtx->D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //set up output merger
    pCtx->D3DContext->OMSetDepthStencilState(pRenderer->defaultDepthStencilState, 0);
    pCtx->D3DContext->OMSetBlendState(pRenderer->defaultBlendState, 0, 0xFFFFFFFF);
    pCtx->D3DContext->RSSetState(pRenderer->defaultRasterizerState);


    ID3D11SamplerState* pSamplers[] = {pRenderer->defaultSampler, mBorderSamplerState, mClampSamplerState, mMotionBlurSamplerState};
    pCtx->D3DContext->PSSetSamplers(0, 4, pSamplers);

    //bind IB and VB for quad rendering
    UINT strides[] = {sizeof(Float3)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &pRenderer->screenQuadVB, strides, offsets);
    pCtx->D3DContext->IASetIndexBuffer(pRenderer->screenQuadIB, DXGI_FORMAT_R32_UINT, 0);
    pCtx->BindShader(&pRenderer->fullScreenQuadVS, 0);
}

void PostProcessRendererD3D11::Leave(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    ID3D11ShaderResourceView* pNullSRVs[] = {0, 0};
    pCtx->D3DContext->PSSetShaderResources(0, 2, pNullSRVs);
}

int PostProcessRendererD3D11::ApplyTonemapping(NFE_CONTEXT_ARG, const ToneMappingDesc& desc)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    RendererTextureD3D11* pSrc = dynamic_cast<RendererTextureD3D11*>(desc.pSource);
    RenderTargetD3D11* pDest = dynamic_cast<RenderTargetD3D11*>(desc.pDest);

    ToneMappingCBuffer toneMapping;
    toneMapping.properties = Float4(desc.exposure, desc.bloomFactor, desc.noiseLevel, desc.saturation);
    toneMapping.outResolution.x = (float)pDest->width;
    toneMapping.outResolution.y = (float)pDest->height;
    toneMapping.outResolution.z = toneMapping.outResolution.w = 0;
    toneMapping.bufferInvRes.x = 1.0f / (float)pRenderer->bufferWidth;
    toneMapping.bufferInvRes.y = 1.0f / (float)pRenderer->bufferHeight;
    toneMapping.bufferInvRes.z = toneMapping.bufferInvRes.w = 0;
    toneMapping.noiseSeed.x = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    toneMapping.noiseSeed.y = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    toneMapping.noiseSeed.z = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    toneMapping.noiseSeed.w = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    pCtx->D3DContext->UpdateSubresource(mToneMappingCBuffer, 0, 0, &toneMapping, 0, 0);


    //bind constant buffer with ambient light
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mToneMappingCBuffer);

    //render target
    pCtx->D3DContext->OMSetRenderTargets(1, &(pDest->RTV), 0);


    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)pDest->width;
    viewport.Height = (float)pDest->height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    ID3D11ShaderResourceView* pSRVs[3] = {0};
    pSRVs[0] = pSrc->SRV;
    pSRVs[2] = pRenderer->noiseTexture->SRV;

    //set source
    pCtx->D3DContext->PSSetShaderResources(0, 3, pSRVs);

    uint32 macros[3] = {0};

    if (desc.pBloom)
    {
        macros[0] = 1;
        pSRVs[1] = (dynamic_cast<RendererTextureD3D11*>(desc.pBloom))->SRV;
    }

    macros[1] = pRenderer->settings.gammaCorrection ? 1 : 0;
    macros[2] = desc.noiseEnabled;

    pCtx->D3DContext->PSSetShaderResources(0, 3, pSRVs);

    ID3D11SamplerState* pSamplers[] = {mClampSamplerState, pRenderer->defaultSampler};
    pCtx->D3DContext->PSSetSamplers(0, 2, pSamplers);

    //bind shaders
    pCtx->BindShader(&mToneMappingPS, macros);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView* pNullSRVs[] = {0};
    pCtx->D3DContext->PSSetShaderResources(0, 1, pNullSRVs);

    return 0;
}

int PostProcessRendererD3D11::Downsaple(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
                                        IRendererTexture* pSource, IRenderTarget* pDest)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    RendererTextureD3D11* pSrc = dynamic_cast<RendererTextureD3D11*>(pSource);

    Float4 inResolution;
    inResolution.x = 1.0f / (float)pSrc->width;
    inResolution.y = 1.0f / (float)pSrc->height;
    inResolution.z = inResolution.w = 0;
    pCtx->D3DContext->UpdateSubresource(mDownsamplingCBuffer, 0, 0, &inResolution, 0, 0);
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mDownsamplingCBuffer);

    //render target
    pCtx->D3DContext->OMSetRenderTargets(1, &(dynamic_cast<RenderTargetD3D11*>(pDest))->RTV, 0);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)srcWidth / 4.0f;
    viewport.Height = (float)srcHeight / 4.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;


    //set source
    pCtx->D3DContext->PSSetShaderResources(0, 1, &pSrc->SRV);

    //bind shaders
    pCtx->BindShader(&mDownsamplingPS, 0);
    pCtx->D3DContext->PSSetSamplers(0, 1, &mClampSamplerState);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView* pNullSRVs[] = {0};
    pCtx->D3DContext->PSSetShaderResources(0, 1, pNullSRVs);

    ID3D11RenderTargetView* pNullRTVs[] = {0};
    pCtx->D3DContext->OMSetRenderTargets(1, pNullRTVs, 0);
    return 0;
}

float sqr(float x)
{
    return x * x;
}

int PostProcessRendererD3D11::Blur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
                                   IRendererTexture* pSource, IRenderTarget* pDest, UINT Mode)
{
    RendererTextureD3D11* pSrc = dynamic_cast<RendererTextureD3D11*>(pSource);
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    //send blur info
    BlurCBuffer cBufferData;
    ZeroMemory(&cBufferData, sizeof(cBufferData));
    cBufferData.inputRes[0] = srcWidth;
    cBufferData.inputRes[1] = srcHeight;
    cBufferData.inputRes[2] = cBufferData.inputRes[3] = 0;
    cBufferData.params.x = (float)pRenderer->settings.bloomSize;
    cBufferData.params.y = 0.5f / sqr(pRenderer->settings.bloomVariance);
    cBufferData.params.z = cBufferData.params.w = 0.0f;
    pCtx->D3DContext->UpdateSubresource(mBlurCBuffer, 0, 0, &cBufferData, 0, 0);


    /*
    if (pRenderer->mFeatureLevel == D3D_FEATURE_LEVEL_11_0)
    {
        pCtx->D3DContext->CSSetUnorderedAccessViews(0, 1, &(pDest->UAV), 0);

        pCtx->D3DContext->CSSetShaderResources(0, 1, &pSource->SRV);
        pCtx->D3DContext->CSSetConstantBuffers(1, 1, &mBlurCBuffer);
        pCtx->BindShader(&mBlurCS, &Mode);

        //uint32 groupsX =  (srcWidth / 32) + ((srcWidth % 32) > 0);
        //uint32 groupsY =  (srcHeight / 32) + ((srcHeight % 32) > 0);
        //pCtx->D3DContext->Dispatch(groupsX, groupsY, 1);


        if (Mode == 0) pCtx->D3DContext->Dispatch(1, srcHeight, 1);
        else pCtx->D3DContext->Dispatch(srcWidth, 1, 1);

        ID3D11UnorderedAccessView* pNullUAVs[] = {0};
        ID3D11ShaderResourceView* pNullSRVs[] = {0};
        pCtx->D3DContext->CSSetUnorderedAccessViews(0, 1, pNullUAVs, 0);
        pCtx->D3DContext->CSSetShaderResources(0, 1, pNullSRVs);
    }
    else
    {*/
    //render target
    pCtx->D3DContext->OMSetRenderTargets(1, &(dynamic_cast<RenderTargetD3D11*>(pDest))->RTV, 0);

    //set source & weights table
    pCtx->D3DContext->PSSetShaderResources(0, 1, &pSrc->SRV);
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mBlurCBuffer);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)srcWidth;
    viewport.Height = (float)srcHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    //bind shader
    pCtx->BindShader(&mBlurPS, &Mode);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView* pNullSRVs[] = {0};
    pCtx->D3DContext->PSSetShaderResources(0, 1, pNullSRVs);
//  }

    return 0;
}

int PostProcessRendererD3D11::AverageTexture(NFE_CONTEXT_ARG, IRendererTexture* pSource,
        uint32 width, uint32 height)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    RendererTextureD3D11* pSrc = dynamic_cast<RendererTextureD3D11*>(pSource);

    uint32 resolution[4];
    resolution[0] = width;
    resolution[1] = height;
    resolution[2] = resolution[3] = 0;
    pCtx->D3DContext->UpdateSubresource(mDownsamplingCBuffer, 0, 0, &resolution, 0, 0);
    pCtx->D3DContext->CSSetConstantBuffers(0, 1, &mDownsamplingCBuffer);

    //set source
    pCtx->D3DContext->CSSetShaderResources(0, 1, &pSrc->SRV);
    pCtx->D3DContext->CSSetUnorderedAccessViews(0, 1, &mAveragingBufferUAV, 0);

    //run compute shaders
    pCtx->BindShader(&mAveragingCS, 0);
    pCtx->D3DContext->Dispatch(1, height, 1);

    //copy results to CPU-readable buffer
    pCtx->D3DContext->CopyResource(mAveragingBuffer, mTempAveragingBuffer);

    ID3D11ShaderResourceView* pNullSRVs[] = {0};
    pCtx->D3DContext->CSSetShaderResources(0, 1, pNullSRVs);
    return 0;
}

//obtain results from AverageTexture()
float PostProcessRendererD3D11::GetAverageColor(NFE_CONTEXT_ARG, uint32 width, uint32 height)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    float result = 0.0f;
    D3D11_MAPPED_SUBRESOURCE mapped;
    pCtx->D3DContext->Map(mAveragingBuffer, 0, D3D11_MAP_READ, 0, &mapped);
    for (uint32 i = 0; i < height; i++)
    {
        result += ((float*)mapped.pData)[i];
    }

    pCtx->D3DContext->Unmap(mAveragingBuffer, 0);

    result /= (float)(width * height);
    return result;
}

int PostProcessRendererD3D11::ApplyAntialiasing(NFE_CONTEXT_ARG, IRendererTexture* pSource,
        IRenderTarget* pDestination)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    RendererTextureD3D11* pSrc = dynamic_cast<RendererTextureD3D11*>(pSource);
    RenderTargetD3D11* pDest = dynamic_cast<RenderTargetD3D11*>(pDestination);

    /*
    MotionBlurCBuffer cbuffer;
    cbuffer.factor = Float4(dt, 0, 0, 0);
    cbuffer.outResolution.x = (float)srcWidth;
    cbuffer.outResolution.y = (float)srcHeight;
    cbuffer.outResolution.z = cbuffer.outResolution.w = 0;
    cbuffer.bufferInvRes.x = 1.0f / (float)pRenderer->mBufferWidth;
    cbuffer.bufferInvRes.y = 1.0f / (float)pRenderer->mBufferHeight;
    cbuffer.bufferInvRes.z = cbuffer.bufferInvRes.w = 0;
    cbuffer.seed[0] = (rand()%RAND_MAX) + ((rand()%RAND_MAX) << 15);
    cbuffer.seed[1] = cbuffer.seed[2] = cbuffer.seed[3] = 0;
    pCtx->D3DContext->UpdateSubresource(mMotionBlurCBuffer, 0, 0, &cbuffer, 0, 0);

    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mMotionBlurCBuffer);
    */

    //bind constant buffer with ambient light
    //render target
    pCtx->D3DContext->OMSetRenderTargets(1, &pDest->RTV, 0);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)pDest->width;
    viewport.Height = (float)pDest->height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    //set source
    pCtx->D3DContext->PSSetShaderResources(0, 1, &pSrc->SRV);
    pCtx->D3DContext->PSSetShaderResources(1, 1, &pRenderer->geomBuffer.SRVs[4]);

    //bind shaders
    pCtx->BindShader(&mAntialiasPS, 0);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView* pNullSRVs[] = {0};
    pCtx->D3DContext->PSSetShaderResources(0, 1, pNullSRVs);

    return 0;
}

int PostProcessRendererD3D11::ApplyFXAA(NFE_CONTEXT_ARG, const FXAADesc& desc)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    RendererTextureD3D11* pSrc = dynamic_cast<RendererTextureD3D11*>(desc.pSource);
    RenderTargetD3D11* pDest = dynamic_cast<RenderTargetD3D11*>(desc.pDest);

    FXAACBuffer fxaa;
    fxaa.properties = Float4(desc.noiseLevel, desc.spanMax, 1.0f / desc.reduceMul,
                             1.0f / desc.reduceMin);
    fxaa.outResolution.x = (float)pDest->width;
    fxaa.outResolution.y = (float)pDest->height;
    fxaa.outResolution.z = fxaa.outResolution.w = 0;
    fxaa.bufferInvRes.x = 1.0f / (float)pRenderer->bufferWidth;
    fxaa.bufferInvRes.y = 1.0f / (float)pRenderer->bufferHeight;
    fxaa.bufferInvRes.z = fxaa.bufferInvRes.w = 0;
    fxaa.noiseSeed.x = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    fxaa.noiseSeed.y = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    fxaa.noiseSeed.z = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    fxaa.noiseSeed.w = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
    pCtx->D3DContext->UpdateSubresource(mFXAACBuffer, 0, 0, &fxaa, 0, 0);

    //bind constant buffer with ambient light
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mFXAACBuffer);

    //render target
    pCtx->D3DContext->OMSetRenderTargets(1, &(pDest->RTV), 0);


    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)pDest->width;
    viewport.Height = (float)pDest->height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    ID3D11ShaderResourceView* pSRVs[2] = {0};
    pSRVs[0] = pSrc->SRV;
    pSRVs[1] = pRenderer->noiseTexture->SRV;

    //set source
    pCtx->D3DContext->PSSetShaderResources(0, 2, pSRVs);

    ID3D11SamplerState* pSamplers[] = {mClampSamplerState, pRenderer->defaultSampler};
    pCtx->D3DContext->PSSetSamplers(0, 2, pSamplers);

    //bind shaders
    uint32 macros[3] = {0};
    macros[0] = 1; //Antialias
    macros[1] = desc.lumaOpt ? 1 : 0;
    macros[2] = desc.noiseEnabled;

    pCtx->BindShader(&mFXAAPS, macros);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView* pNullSRVs[] = {0};
    pCtx->D3DContext->PSSetShaderResources(0, 1, pNullSRVs);

    return 0;
}

//this function uses GBuffer from pRenderer!
int PostProcessRendererD3D11::ApplyMotionBlur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
        IRendererTexture* pSource, IRenderTarget* pDestination, float dt)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    RendererTextureD3D11* pSrc = dynamic_cast<RendererTextureD3D11*>(pSource);
    RenderTargetD3D11* pDest = dynamic_cast<RenderTargetD3D11*>(pDestination);

    MotionBlurCBuffer cbuffer;
    cbuffer.factor = Float4(dt, 0, 0, 0);
    cbuffer.outResolution.x = (float)srcWidth;
    cbuffer.outResolution.y = (float)srcHeight;
    cbuffer.outResolution.z = cbuffer.outResolution.w = 0;
    cbuffer.bufferInvRes.x = 1.0f / (float)pRenderer->bufferWidth;
    cbuffer.bufferInvRes.y = 1.0f / (float)pRenderer->bufferHeight;
    cbuffer.bufferInvRes.z = cbuffer.bufferInvRes.w = 0;
    cbuffer.seed[0] = (rand() % RAND_MAX) + ((rand() % RAND_MAX) << 15);
    cbuffer.seed[1] = cbuffer.seed[2] = cbuffer.seed[3] = 0;
    pCtx->D3DContext->UpdateSubresource(mMotionBlurCBuffer, 0, 0, &cbuffer, 0, 0);


    //bind constant buffer with ambient light
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mMotionBlurCBuffer);

    //render target
    pCtx->D3DContext->OMSetRenderTargets(1, &pDest->RTV, 0);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)pDest->width;
    viewport.Height = (float)pDest->height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    //set source
    pCtx->D3DContext->PSSetShaderResources(0, 1, &pSrc->SRV);
    pCtx->D3DContext->PSSetShaderResources(1, 1,
                                           &pRenderer->geomBuffer.SRVs[3]); //motion buffer

    //bind shaders
    pCtx->BindShader(&mMotionBlurPS, 0);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView* pNullSRVs[] = {0};
    pCtx->D3DContext->PSSetShaderResources(0, 1, pNullSRVs);

    return 0;
}

} // namespace Render
} // namespace NFE
