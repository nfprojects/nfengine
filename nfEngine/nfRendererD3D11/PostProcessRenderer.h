#pragma once
#include "nfRendererD3D11.h"
#include "Multishader.h"
#include "../nfCore/Renderer.h"

namespace NFE {
namespace Render {

struct ToneMappingDesc
{
    IRendererTexture* pSource;
    IRendererTexture* pBloom;
    IRenderTarget* pDest;

    float exposure;
    float bloomFactor;
    float noiseLevel;
    bool  noiseEnabled;
    float saturation;
};

struct FXAADesc
{
    IRendererTexture* pSource;
    IRenderTarget* pDest;

    float noiseLevel;
    bool  noiseEnabled;

    bool lumaOpt;
    float spanMax;
    float reduceMul;
    float reduceMin;
};

// TODO: move to Renderer.h
class IPostProcessRenderer
{
public:
    virtual ~IPostProcessRenderer() {};

    virtual void Enter(NFE_CONTEXT_ARG) = 0;
    virtual void Leave(NFE_CONTEXT_ARG) = 0;

    virtual int ApplyAntialiasing(NFE_CONTEXT_ARG, IRendererTexture* pSource,
                                  IRenderTarget* pDestination) = 0;
    virtual int ApplyFXAA(NFE_CONTEXT_ARG, const FXAADesc& desc) = 0;

    virtual int ApplyTonemapping(NFE_CONTEXT_ARG, const ToneMappingDesc& desc) = 0;
    virtual int Downsaple(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight, IRendererTexture* pSource,
                          IRenderTarget* pDest) = 0;
    virtual int Blur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight, IRendererTexture* pSource,
                     IRenderTarget* pDest, UINT Mode) = 0;
    virtual int AverageTexture(NFE_CONTEXT_ARG, IRendererTexture* pSource, uint32 width,
                               uint32 height) = 0;
    virtual float GetAverageColor(NFE_CONTEXT_ARG, uint32 width, uint32 height) = 0;

    virtual int ApplyMotionBlur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
                                IRendererTexture* pSource, IRenderTarget* pDestination, float dt) = 0;
};

class PostProcessRendererD3D11 : public IPostProcessRenderer
{
    ID3D11Buffer* mToneMappingCBuffer;
    ID3D11Buffer* mFXAACBuffer;
    ID3D11Buffer* mDownsamplingCBuffer;
    ID3D11Buffer* mMotionBlurCBuffer;
    ID3D11Buffer* mBlurCBuffer;

    ID3D11Buffer* mTempAveragingBuffer;
    ID3D11UnorderedAccessView* mAveragingBufferUAV;
    ID3D11Buffer* mAveragingBuffer;

    ID3D11SamplerState* mBorderSamplerState;
    ID3D11SamplerState* mClampSamplerState;
    ID3D11SamplerState* mMotionBlurSamplerState;

    Multishader mToneMappingPS;
    Multishader mDownsamplingPS;
    Multishader mBlurPS;
    Multishader mBlurCS;
    Multishader mMotionBlurPS;
    Multishader mAveragingCS;
    Multishader mAntialiasPS;
    Multishader mFXAAPS;

public:
    PostProcessRendererD3D11();
    ~PostProcessRendererD3D11();

    Result Init(RendererD3D11* pRenderer);
    void Release();
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    int ApplyAntialiasing(NFE_CONTEXT_ARG, IRendererTexture* pSource, IRenderTarget* pDestination);
    int ApplyFXAA(NFE_CONTEXT_ARG, const FXAADesc& desc);

    int ApplyTonemapping(NFE_CONTEXT_ARG, const ToneMappingDesc& desc);
    int Downsaple(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight, IRendererTexture* pSource,
                  IRenderTarget* pDest);
    int Blur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight, IRendererTexture* pSource,
             IRenderTarget* pDest, UINT Mode);
    int AverageTexture(NFE_CONTEXT_ARG, IRendererTexture* pSource, uint32 width, uint32 height);
    float GetAverageColor(NFE_CONTEXT_ARG, uint32 width, uint32 height);

    int ApplyMotionBlur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight, IRendererTexture* pSource,
                        IRenderTarget* pDestination, float dt);
};

} // namespace Render
} // namespace NFE
