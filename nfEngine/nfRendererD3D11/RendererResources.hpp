#pragma once
#include "nfRendererD3D11.hpp"
#include "../nfCore/Renderer.hpp"

namespace NFE {
namespace Render {

//structure representing vertex or index buffer resource
class RendererBufferD3D11 : public IRendererBuffer
{
public:
    ID3D11Buffer* D3DBuffer;
    uint32 size;

    RendererBufferD3D11();
    ~RendererBufferD3D11();
    void Release();
    bool Init(Type type, const void* pData, uint32 dataSize);
    ID3D11Buffer* GetBuffer() const;
    uint32 GetSize() const;
};

// helper structure wrapping D3D11_SAMPLER_DESC
struct RendererD3D11SamplerDesc : public D3D11_SAMPLER_DESC
{
    // default sampler desc
    explicit RendererD3D11SamplerDesc( D3D11_FILTER filter = D3D11_FILTER_ANISOTROPIC,
                                       D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_WRAP,
                                       D3D11_COMPARISON_FUNC comparisonFunc = D3D11_COMPARISON_ALWAYS,
                                       UINT maxAnisotropy = 16,
                                       float minLOD = -D3D11_FLOAT32_MAX,
                                       float maxLOD = D3D11_FLOAT32_MAX,
                                       const float* borderColor = nullptr,
                                       float mipLODBias = 0)
    {
        AddressU = address;
        AddressV = address;
        AddressW = address;
        Filter = filter;
        MaxAnisotropy = maxAnisotropy;
        MaxLOD = maxLOD;
        MinLOD = minLOD;
        if (borderColor)
        {
            BorderColor[0] = borderColor[0];
            BorderColor[1] = borderColor[1];
            BorderColor[2] = borderColor[2];
            BorderColor[3] = borderColor[3];
        }
        else
            BorderColor[0] = BorderColor[1] = BorderColor[2] = BorderColor[3] = 1.0f;
        MipLODBias = mipLODBias;
        ComparisonFunc = comparisonFunc;
    }

    // custom sampler desc
    explicit RendererD3D11SamplerDesc(const D3D11_SAMPLER_DESC& desc) : D3D11_SAMPLER_DESC(desc) {};
    operator const D3D11_SAMPLER_DESC& () const
    {
        return *this;
    }
};


struct RendererD3D11DepthStencilDesc : public D3D11_DEPTH_STENCIL_DESC
{
    explicit RendererD3D11DepthStencilDesc( BOOL depthEnable = FALSE,
                                            D3D11_DEPTH_WRITE_MASK depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
                                            D3D11_COMPARISON_FUNC depthFunc = D3D11_COMPARISON_ALWAYS)
    {
        DepthEnable = depthEnable;
        DepthWriteMask = depthWriteMask;
        DepthFunc = depthFunc;
        StencilEnable = FALSE;
        StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
        {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS};
        FrontFace = defaultStencilOp;
        BackFace = defaultStencilOp;
    }

    // custom desc
    explicit RendererD3D11DepthStencilDesc(const D3D11_DEPTH_STENCIL_DESC& desc) :
        D3D11_DEPTH_STENCIL_DESC(desc) {};
    operator const D3D11_DEPTH_STENCIL_DESC& () const
    {
        return *this;
    }
};

//geometry buffer for deferred shading
class GeometryBuffer
{
public:
    static const int mLayersNum = 4;

    ID3D11Texture2D* textures[4];
    ID3D11ShaderResourceView* SRVs[4];
    ID3D11RenderTargetView* RTVs[4];
    uint32 width, height;

    GeometryBuffer();
    ~GeometryBuffer();
    void Release();
    int Resize(uint32 width_, uint32 height_);
};


//shadow map
class ShadowMapD3D11 : public IShadowMap
{
public:
    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* SRV;
    ID3D11RenderTargetView** RTVs;

    //TODO - no need of creating that for every shadow map!
    ID3D11Texture2D* depthTexture;
    ID3D11DepthStencilView* DSV;

    uint32 size;
    uint32 splits;
    Type type;

    ShadowMapD3D11();
    ~ShadowMapD3D11();
    void Release();
    int Resize(uint32 size_, Type type_, uint32 splits_ = 1);
    uint32 GetSize() const;
};

//renderer 2D texture resource
class RendererTextureD3D11 : virtual public IRendererTexture
{
public:
    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* SRV;
    uint32 width, height;

    RendererTextureD3D11();
    virtual ~RendererTextureD3D11();
    virtual void Release();
    virtual Result FromImage(const Common::Image& image);
};

//buffer for view from cameras, tone mapping, etc.
class RenderTargetD3D11 : public RendererTextureD3D11, public IRenderTarget
{
    int InitRenderTarget(uint32 width_, uint32 height_, bool HDR);
    int InitSwapChain(uint32 width_, uint32 height_, Common::Window* pWindow, bool HDR);

public:
    ID3D11RenderTargetView* RTV;
    ID3D11UnorderedAccessView* UAV;
    bool HDR; // use RGBA8 or RGBA16

    //used only for window output
    IDXGISwapChain* swapChain;
    Common::Window* window;

    RenderTargetD3D11();
    virtual ~RenderTargetD3D11();
    virtual void Release();
    virtual int OnResize(); // used only for targets binded with a window
    virtual int Init(uint32 width_, uint32 height_, Common::Window* pWindow = NULL, bool HDR_ = false);
    virtual int Present();
    Result FromImage(const Common::Image& image)
    {
        return Result::OK;
    }
};

} // namespace Render
} // namespace NFE
