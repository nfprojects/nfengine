#include "stdafx.hpp"
#include "RendererResources.hpp"
#include "Renderer.hpp"
#include "../nfCommon/Window.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/Image.hpp"

namespace NFE {
namespace Render {

// --------------------------------------------------------------------------------------------
// RendererTextureD3D11
// --------------------------------------------------------------------------------------------
RendererTextureD3D11::RendererTextureD3D11()
{
    texture = 0;
    SRV = 0;
    width = height = 0;
}

RendererTextureD3D11::~RendererTextureD3D11()
{
    Release();
}

void RendererTextureD3D11::Release()
{
    D3D_SAFE_RELEASE(SRV);
    D3D_SAFE_RELEASE(texture);
}

Result RendererTextureD3D11::FromImage(const Common::Image& image)
{
    using namespace Common;
    RendererD3D11* pRenderer = RendererD3D11::GetInstance();


    HRESULT HR;
    size_t bitsPerPixel = Image::BitsPerPixel(image.GetFormat());

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = image.GetWidth();
    texDesc.Height = image.GetHeight();
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.MipLevels = (UINT)image.GetMipmapsNum();
    texDesc.MiscFlags = 0;

    bool bc = false;
    size_t bcNumBytesPerBlock = 0;
    switch (image.GetFormat())
    {
        case ImageFormat::R_UBYTE:
            texDesc.Format = DXGI_FORMAT_R8_UNORM;
            break;

        case ImageFormat::A_UBYTE:
            texDesc.Format = DXGI_FORMAT_A8_UNORM;
            break;

        case ImageFormat::RGBA_UBYTE:
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;

        case ImageFormat::RGBA_FLOAT:
            texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            break;

        case ImageFormat::R_FLOAT:
            texDesc.Format = DXGI_FORMAT_R32_FLOAT;
            break;

        case ImageFormat::BC1:
            texDesc.Format = DXGI_FORMAT_BC1_UNORM;
            bcNumBytesPerBlock = 8;
            bc = true;
            break;

        case ImageFormat::BC2:
            texDesc.Format = DXGI_FORMAT_BC2_UNORM;
            bcNumBytesPerBlock = 16;
            bc = true;
            break;

        case ImageFormat::BC3:
            texDesc.Format = DXGI_FORMAT_BC3_UNORM;
            bcNumBytesPerBlock = 16;
            bc = true;
            break;

        case ImageFormat::BC4:
            texDesc.Format = DXGI_FORMAT_BC4_UNORM;
            bcNumBytesPerBlock = 8;
            bc = true;
            break;

        case ImageFormat::BC5:
            texDesc.Format = DXGI_FORMAT_BC5_UNORM;
            bcNumBytesPerBlock = 16;
            bc = true;
            break;

        default:
            return Result::Error;
    }

    uint32 mipmapLevels = (uint32)image.GetMipmapsNum();

    // fill D3D11_SUBRESOURCE_DATA table
    D3D11_SUBRESOURCE_DATA* initData = new D3D11_SUBRESOURCE_DATA [mipmapLevels];
    if (initData == 0)
        return Result::AllocationError;

    for (uint32 i = 0; i < mipmapLevels; i++)
    {
        if (bc) //special case - block coding
        {
            uint32 numBlocksWide = std::max<uint32>(1, (image.GetMipmap(i).width + 3) / 4);
            //uint32 numBlocksHigh = std::max<uint32>(1, (image.m_Mipmaps[i].height + 3) / 4);
            initData[i].SysMemPitch = (UINT)(numBlocksWide * bcNumBytesPerBlock);
        }
        else
        {
            initData[i].SysMemPitch = Math::Max<uint32>(1,
                                      static_cast<uint32>(image.GetMipmap(i).width * bitsPerPixel / 8));
        }

        initData[i].pSysMem = image.GetMipmap(i).data;
        initData[i].SysMemSlicePitch = (UINT)(image.GetMipmap(i).dataSize);
    }

    //create direct3d texture object
    HR = D3D_CHECK(pRenderer->D3DDevice->CreateTexture2D(&texDesc, initData, &texture));
    if (FAILED(HR))
    {
        //failed to create d3d texture
        delete[] initData;
        return Result::Error;
    }

    delete[] initData;


    // create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = mipmapLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    HR = D3D_CHECK(pRenderer->D3DDevice->CreateShaderResourceView(texture, &srvDesc, &SRV));
    if (FAILED(HR))
    {
        //failed to create d3d texture
        return Result::Error;
    }

    width = image.GetWidth();
    height = image.GetHeight();

    return Result::OK;
}



// --------------------------------------------------------------------------------------------
// XVertexBuffer
// --------------------------------------------------------------------------------------------
RendererBufferD3D11::RendererBufferD3D11()
{
    D3DBuffer = 0;
    size = 0;
}

RendererBufferD3D11::~RendererBufferD3D11()
{
    Release();
}

void RendererBufferD3D11::Release()
{
    D3D_SAFE_RELEASE(D3DBuffer);
    size = 0;
}

bool RendererBufferD3D11::Init(Type type, const void* pData, uint32 dataSize)
{
    RendererD3D11* pRenderer = RendererD3D11::GetInstance();

    D3D11_BUFFER_DESC bd;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = dataSize;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.Usage = D3D11_USAGE_IMMUTABLE;

    // TODO: dynamic buffers support

    switch (type)
    {
        case Type::Vertex:
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;

        case Type::Index:
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            break;

        default:
            LOG_ERROR("Invalid renderer buffer type.");
            return false;
    }

    D3D11_SUBRESOURCE_DATA sd;
    sd.pSysMem = pData;
    sd.SysMemPitch = 0;
    sd.SysMemSlicePitch = 0;

    HRESULT hr = D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, &sd, &D3DBuffer));

    if (SUCCEEDED(hr))
    {
        size = dataSize;
        return true;
    }

    return false;
}

ID3D11Buffer* RendererBufferD3D11::GetBuffer() const
{
    return D3DBuffer;
}

uint32 RendererBufferD3D11::GetSize() const
{
    return size;
}



// --------------------------------------------------------------------------------------------
// GeometryBuffer
// --------------------------------------------------------------------------------------------
GeometryBuffer::GeometryBuffer()
{
    for (int i = 0; i < mLayersNum; i++)
    {
        textures[i] = 0;
        SRVs[i] = 0;
        RTVs[i] = 0;
    }
    width = 0;
    height = 0;
}

GeometryBuffer::~GeometryBuffer()
{
    Release();
}

void GeometryBuffer::Release()
{
    for (int i = 0; i < mLayersNum; i++)
    {
        D3D_SAFE_RELEASE(SRVs[i]);
        D3D_SAFE_RELEASE(RTVs[i]);
        D3D_SAFE_RELEASE(textures[i]);
    }
    width = 0;
    height = 0;
}

int GeometryBuffer::Resize(uint32 width_, uint32 height_)
{
    RendererD3D11* pRenderer = RendererD3D11::GetInstance();

    if ((width == width_) && (height == height_))
    {
        //gbuffer dimensions are the same - do nothing
        return 0;
    }

    Release();

    //texture descriptor
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = width_;
    texDesc.Height = height_;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.MipLevels = 1;
    texDesc.MiscFlags = 0;
    texDesc.CPUAccessFlags = 0;

    //shader resource view descriptor
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    //render target view descriptor
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;


    //format of each gbuffer texture
    DXGI_FORMAT formats[] = { DXGI_FORMAT_R16G16B16A16_FLOAT,
                              DXGI_FORMAT_R16G16B16A16_FLOAT,
                              DXGI_FORMAT_R16G16B16A16_FLOAT,
                              DXGI_FORMAT_R16G16_FLOAT
                            };

    bool ErrorOccurred = false;
    HRESULT HR;
    for (int i = 0; i < mLayersNum; i++)
    {
        texDesc.Format = formats[i];
        HR = D3D_CHECK(pRenderer->D3DDevice->CreateTexture2D(&texDesc, 0, &(textures[i])));
        if (FAILED(HR))
        {
            ErrorOccurred = true;
            break;
        }

        srvDesc.Format = formats[i];
        HR = D3D_CHECK(pRenderer->D3DDevice->CreateShaderResourceView(textures[i], &srvDesc,
                       &(SRVs[i])));
        if (FAILED(HR))
        {
            ErrorOccurred = true;
            break;
        }

        rtvDesc.Format = formats[i];
        HR = D3D_CHECK(pRenderer->D3DDevice->CreateRenderTargetView(textures[i], &rtvDesc,
                       &(RTVs[i])));
        if (FAILED(HR))
        {
            ErrorOccurred = true;
            break;
        }
    }

    if (ErrorOccurred)
    {
        Release();
        return 1;
    }

    //success
    width = width_;
    height = height_;
    return 0;
}


// --------------------------------------------------------------------------------------------
// ShadowMapD3D11
// --------------------------------------------------------------------------------------------
ShadowMapD3D11::ShadowMapD3D11()
{
    texture = 0;
    SRV = 0;
    RTVs = 0;

    depthTexture = 0;
    DSV = 0;

    size = 0;
    splits = 0;
    type = Type::None;
}

ShadowMapD3D11::~ShadowMapD3D11()
{
    Release();
}

void ShadowMapD3D11::Release()
{
    if (RTVs)
    {
        for (uint32 i = 0; i < splits; i++)
        {
            D3D_SAFE_RELEASE(RTVs[i]);
        }

        free(RTVs);
        RTVs = 0;
    }

    D3D_SAFE_RELEASE(SRV);
    D3D_SAFE_RELEASE(texture);

    D3D_SAFE_RELEASE(DSV);
    D3D_SAFE_RELEASE(depthTexture);

    size = 0;
    splits = 0;
    type = Type::None;
}

uint32 ShadowMapD3D11::GetSize() const
{
    return size;
}

int ShadowMapD3D11::Resize(uint32 size_, Type type_, uint32 splits_)
{
    RendererD3D11* pRenderer = RendererD3D11::GetInstance();


    if ((size == size_) && (splits == splits_) && (type_ == type))
    {
        //shadow map dimensions are the same - do nothing
        return 0;
    }

    Release();

    if (size_ == 0) return 0;

    switch (type_)
    {
        case Type::Flat:
        {
            splits_ = 1;
            break;
        }

        case Type::Cascaded:
        {
            if (splits_ > 8) splits_ = 8;
            if (splits_ < 1) splits_ = 1;
            break;
        }

        case Type::Cube:
        {
            splits_ = 6;
            break;
        }

        default:
            return 0;
    };

    RTVs = (ID3D11RenderTargetView**)malloc(splits_ * sizeof(ID3D11RenderTargetView*));

    HRESULT HR;

    //texture descriptor
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = size_;
    texDesc.Height = size_;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.ArraySize = splits_;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.MipLevels = 1;
    texDesc.CPUAccessFlags = 0;
    texDesc.Format = DXGI_FORMAT_R32_FLOAT;
    texDesc.MiscFlags = (type_ == Type::Cube) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

    //create texture object
    HR = D3D_CHECK(pRenderer->D3DDevice->CreateTexture2D(&texDesc, 0, &texture));
    if (FAILED(HR))
    {
        Release();
        return 1;
    }


    //shader resource view descriptor
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    switch (type_)
    {
        case Type::Flat:
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            break;

        case Type::Cascaded:
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.ArraySize = splits_;
            srvDesc.Texture2DArray.FirstArraySlice = 0;
            srvDesc.Texture2DArray.MipLevels = 1;
            srvDesc.Texture2DArray.MostDetailedMip = 0;
            break;

        case Type::Cube:
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MipLevels = 1;
            srvDesc.TextureCube.MostDetailedMip = 0;
            break;
    };

    //create shader resource view for cube texture
    HR = D3D_CHECK(pRenderer->D3DDevice->CreateShaderResourceView(texture, &srvDesc, &SRV));
    if (FAILED(HR))
    {
        Release();
        return 1;
    }


    //render target view descriptor
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;

    if (type_ == Type::Flat)
    {
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        HR = D3D_CHECK(pRenderer->D3DDevice->CreateRenderTargetView(texture, &rtvDesc,
                       &(RTVs[0])));
        if (FAILED(HR))
        {
            Release();
            return 1;
        }
    }
    else
    {
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = 0;
        rtvDesc.Texture2DArray.ArraySize = 1;

        //create render target view for each cube face / cascade
        for (uint32 i = 0; i < splits_; i++)
        {
            rtvDesc.Texture2DArray.FirstArraySlice = i;
            HR = D3D_CHECK(pRenderer->D3DDevice->CreateRenderTargetView(texture, &rtvDesc,
                           &(RTVs[i])));
            if (FAILED(HR))
            {
                Release();
                return 1;
            }
        }
    }


    //description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = size_;
    depthBufferDesc.Height = size_;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    //create the texture for the depth buffer
    HR = D3D_CHECK(pRenderer->D3DDevice->CreateTexture2D(&depthBufferDesc, NULL, &depthTexture));
    if (FAILED(HR))
    {
        Release();
        return 1;
    }


    // Set up the depth stencil view description.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    HR = D3D_CHECK(pRenderer->D3DDevice->CreateDepthStencilView(depthTexture,
                   &depthStencilViewDesc, &DSV));
    if (FAILED(HR))
    {
        Release();
        return 1;
    }

    type = type_;
    splits = splits_;
    size = size_;
    return 0;
}



void XEngine_OnWindowResizeD3D11(void* pUserData)
{
    auto pRT = (RenderTargetD3D11*)pUserData;
    pRT->OnResize();
}


// --------------------------------------------------------------------------------------------
// RenderTargetD3D11
// --------------------------------------------------------------------------------------------
RenderTargetD3D11::RenderTargetD3D11()
{
    texture = NULL;
    SRV = NULL;
    RTV = NULL;
    UAV = NULL;
    width = 0;
    height = 0;
    HDR = false;

    swapChain = NULL;
    window = NULL;
}

RenderTargetD3D11::~RenderTargetD3D11()
{
    Release();
}

void RenderTargetD3D11::Release()
{
    D3D_SAFE_RELEASE(UAV);
    D3D_SAFE_RELEASE(RTV);
    D3D_SAFE_RELEASE(SRV);
    D3D_SAFE_RELEASE(texture);
    D3D_SAFE_RELEASE(swapChain);

    width = 0;
    height = 0;

    if (window)
    {
        window->setResizeCallback(NULL, this);
        window = NULL;
    }
}

int RenderTargetD3D11::Init(uint32 width_, uint32 height_, Common::Window* pWindow, bool HDR_)
{
    if ((width == width_) && (height == height_) && (HDR == HDR_) && (window == pWindow))
    {
        // dimensions are the same - do nothing
        return 0;
    }

    Release();

    int ret = 0;

    if (pWindow != NULL)
        ret = InitSwapChain(width_, height_, pWindow, HDR_);
    else
        ret = InitRenderTarget(width_, height_, HDR_);

    return ret;
}

int RenderTargetD3D11::InitRenderTarget(uint32 width_, uint32 height_, bool HDR_)
{
    RendererD3D11* pRenderer = RendererD3D11::GetInstance();

    //texture descriptor
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = width_;
    texDesc.Height = height_;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    if (pRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0) texDesc.BindFlags |=
            D3D11_BIND_UNORDERED_ACCESS;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.MipLevels = 0;
    texDesc.MiscFlags = 0;
    texDesc.CPUAccessFlags = 0;
    texDesc.Format = HDR_ ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;

    if (pRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0)
        texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    //shader resource view descriptor
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = (UINT)(-1);
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Format = texDesc.Format;

    //render target view descriptor
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Format = texDesc.Format;

    //unordered access view descriptor
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Format = texDesc.Format;

    bool ErrorOccurred = false;
    HRESULT HR;

    HR = D3D_CHECK(pRenderer->D3DDevice->CreateTexture2D(&texDesc, 0, &texture));
    texture->SetPrivateData(WKPDID_D3DDebugObjectName, 5, "blah");
    if (FAILED(HR))
        ErrorOccurred = true;

    HR = D3D_CHECK(pRenderer->D3DDevice->CreateShaderResourceView(texture, &srvDesc, &SRV));
    if (FAILED(HR))
        ErrorOccurred = true;

    HR = D3D_CHECK(pRenderer->D3DDevice->CreateRenderTargetView(texture, &rtvDesc, &RTV));
    if (FAILED(HR))
        ErrorOccurred = true;

    if (pRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0)
    {
        D3D_CHECK(pRenderer->D3DDevice->CreateUnorderedAccessView(texture, &uavDesc, &UAV));
    }

    if (ErrorOccurred)
    {
        Release();
        return 1;
    }

    //success
    width = width_;
    height = height_;
    HDR = HDR_;

    return 0;
}

int RenderTargetD3D11::InitSwapChain(uint32 width_, uint32 height_, Common::Window* pWindow,
                                     bool HDR_)
{
    RendererD3D11* pRenderer = RendererD3D11::GetInstance();
    HRESULT hr;

    pWindow->setResizeCallback(XEngine_OnWindowResizeD3D11, this);
    window = pWindow;
    width = width_;
    height = height_;

    DXGI_USAGE usageFlags = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    if (pRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0)
        usageFlags |= DXGI_USAGE_UNORDERED_ACCESS;

    // create swap chain
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width_;
    scd.BufferDesc.Height = height_;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // use 32-bit color
    scd.BufferUsage = usageFlags;
    scd.OutputWindow = pWindow->getHandle();
    scd.SampleDesc.Count = 1;                           // how many multisamples
    scd.Windowed = !pWindow->getFullscreenMode();       // windowed/full-screen mode

    hr = D3D_CHECK(pRenderer->DXGIFactory->CreateSwapChain(pRenderer->D3DDevice, &scd,
                   &swapChain));
    if (FAILED(hr))
    {
        LOG_FATAL("Could not create swap chain!");
        return 1;
    }

    // get the address of the back buffer
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture);
    if (FAILED(hr))
    {
        LOG_FATAL("Could not get device back buffer!");
        return 1;
    }

    hr = D3D_CHECK(pRenderer->D3DDevice->CreateRenderTargetView(texture, NULL, &RTV));
    if (FAILED(hr)) return 1;

    if (pRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0)
    {
        hr = D3D_CHECK(pRenderer->D3DDevice->CreateUnorderedAccessView(texture, NULL, &UAV));

        if (FAILED(hr)) return 1;
    }

    return 0;
}

int RenderTargetD3D11::Present()
{
    if (swapChain)
    {
        // TODO: vsync must be per-display
        return SUCCEEDED(swapChain->Present(0, 0)) ? 0 : 1;
    }

    return 0;
}

int RenderTargetD3D11::OnResize()
{
    RendererD3D11* pRenderer = RendererD3D11::GetInstance();

    //no size change
    UINT width_, height_;
    window->getSize(width_, height_);
    if (width_ == width && height_ == height)
    {
        return 1;
    }

    width = width_;
    height = height_;

    if (width == 0 || height == 0)
        return 1;

    HRESULT hr;
    if (!swapChain)
        return 1;

    // Release all outstanding references to the swap chain's buffers.
    //((RenderContextD3D11*)g_pImmediateContext)->D3DContext->OMSetRenderTargets(0, 0, 0);
    D3D_SAFE_RELEASE(RTV);
    D3D_SAFE_RELEASE(UAV);
    D3D_SAFE_RELEASE(texture);

    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    //swapChain->SetFullscreenState(m_pTargetWindow->m_Fullscreen, 0);

    if (FAILED(hr))
    {
        LOG_ERROR("Failed to resize swap chain.");
        return 1;
    }

    // Get buffer and create a render-target-view.
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &texture);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to obtain swap chain's buffer.");
        return 1;
    }

    hr = D3D_CHECK(pRenderer->D3DDevice->CreateRenderTargetView(texture, NULL, &RTV));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create render target view.");
        return 1;
    }

    if (pRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0)
    {
        hr = D3D_CHECK(pRenderer->D3DDevice->CreateUnorderedAccessView(texture, NULL, &UAV));

        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create unordered access view.");
            return 1;
        }
    }

    return 0;
}

} // namespace Render
} // namespace NFE
