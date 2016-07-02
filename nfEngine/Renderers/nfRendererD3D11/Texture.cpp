/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's texture
 */

#include "PCH.hpp"
#include "Texture.hpp"
#include "RendererD3D11.hpp"
#include "Translations.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
{
    mTextureGeneric = nullptr;
    mType = TextureType::Unknown;
}

Texture::~Texture()
{
    switch (mType)
    {
    case TextureType::Texture1D:
        D3D_SAFE_RELEASE(mTexture1D);
        break;
    case TextureType::Texture2D:
    case TextureType::TextureCube:
        D3D_SAFE_RELEASE(mTexture2D);
        break;
    case TextureType::Texture3D:
        D3D_SAFE_RELEASE(mTexture3D);
        break;
    };
}

bool Texture::InitTexture1D(const TextureDesc& desc)
{
    // TODO: fill
    UNUSED(desc);

    mType = TextureType::Texture1D;
    return true;
}

bool Texture::InitTexture2D(const TextureDesc& desc)
{
    HRESULT hr;

    if (desc.width < 1 || desc.height < 1 || desc.depth < 1)
    {
        LOG_ERROR("Invalid texture dimensions (width = %i, height = %i, depth = %i)",
                  desc.width, desc.height, desc.depth);
        return false;
    }


    if (desc.binding & ~(NFE_RENDERER_TEXTURE_BIND_SHADER |
                         NFE_RENDERER_TEXTURE_BIND_RENDERTARGET |
                         NFE_RENDERER_TEXTURE_BIND_DEPTH))
    {
        LOG_ERROR("Invalid texture binding flags");
        return false;
    }


    DXGI_FORMAT resourceFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
    {
        if (!TranslateDepthBufferTypes(desc.depthBufferFormat,
                                       resourceFormat, srvFormat, dsvFormat))
        {
            LOG_ERROR("Invalid depth buffer format");
            return false;
        }
    }
    else
    {
        resourceFormat = TranslateElementFormat(desc.format, desc.texelSize);
        srvFormat = resourceFormat;
        if (resourceFormat == DXGI_FORMAT_UNKNOWN)
        {
            LOG_ERROR("Invalid texture format");
            return false;
        }
    }

    D3D11_TEXTURE2D_DESC td;
    td.Width = desc.width;
    td.Height = desc.height;
    td.ArraySize = desc.layers;
    td.BindFlags = 0;
    td.CPUAccessFlags = 0; // TODO: support for dynamic textures
    td.Format = resourceFormat;
    td.MipLevels = desc.mipmaps;
    td.MiscFlags = 0;
    if (desc.type == TextureType::TextureCube)
        td.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    td.SampleDesc.Count = desc.samplesNum;
    td.SampleDesc.Quality = 0;

    switch (desc.access)
    {
        case BufferAccess::CPU_Read:
            td.Usage = D3D11_USAGE_STAGING;
            td.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
            break;
        case BufferAccess::CPU_Write:
            td.Usage = D3D11_USAGE_DYNAMIC;
            break;
        case BufferAccess::GPU_ReadOnly:
            td.Usage = D3D11_USAGE_IMMUTABLE;
            break;
        case BufferAccess::GPU_ReadWrite:
            td.Usage = D3D11_USAGE_DEFAULT;
            break;
        default:
            LOG_ERROR("Invalid texture access mode");
            return false;
    };

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
        td.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_RENDERTARGET)
        td.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
        td.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    // TODO: UAV support

    if (desc.dataDesc)
    {
        D3D11_SUBRESOURCE_DATA initialData[MAX_MIPMAP_LEVELS];
        for (int i = 0; i < desc.mipmaps; ++i)
        {
            initialData[i].pSysMem = desc.dataDesc[i].data;
            initialData[i].SysMemPitch = static_cast<UINT>(desc.dataDesc[i].lineSize);
            initialData[i].SysMemSlicePitch = static_cast<UINT>(desc.dataDesc[i].sliceSize);
        }

        hr = D3D_CALL_CHECK(gDevice->Get()->CreateTexture2D(&td, initialData, &mTexture2D));
    }
    else
    {
        if (desc.access == BufferAccess::GPU_ReadOnly)
        {
            LOG_ERROR("GPU read-only textures must be created with initial data provided");
            return false;
        }

        hr = D3D_CALL_CHECK(gDevice->Get()->CreateTexture2D(&td, NULL, &mTexture2D));
    }


    if (FAILED(hr))
        return false;


#ifdef D3D_DEBUGGING
    /// set debug name
    std::string textureName = "NFE::Renderer::Texture \"";
    if (desc.debugName)
        textureName += desc.debugName;
    textureName += '"';
    mTexture2D->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(textureName.length()),
                               textureName.c_str());
#endif // D3D_DEBUGGING


    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
        ZeroMemory(&srvd, sizeof(srvd));
        srvd.Format = srvFormat;

        if (desc.type == TextureType::TextureCube)
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvd.TextureCube.MipLevels = desc.mipmaps;
            srvd.TextureCube.MostDetailedMip = 0;
        }
        else if (desc.samplesNum > 1)
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels = desc.mipmaps;
            srvd.Texture2D.MostDetailedMip = 0;
        }

        hr = D3D_CALL_CHECK(gDevice->Get()->CreateShaderResourceView(mTexture2D, &srvd, &mSRV));
        if (FAILED(hr))
        {
            D3D_SAFE_RELEASE(mTexture2D);
            return false;
        }

#ifdef D3D_DEBUGGING
        // set debug name
        mSRV->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(textureName.length()),
                             textureName.c_str());
#endif // D3D_DEBUGGING
    }

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
        dsvd.Flags = 0;
        dsvd.Format = dsvFormat;
        if (desc.samplesNum > 1)
        {
            dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvd.Texture2D.MipSlice = 0;
        }

        hr = D3D_CALL_CHECK(gDevice->Get()->CreateDepthStencilView(mTexture2D, &dsvd, &mDSV));
        if (FAILED(hr))
        {
            mSRV.reset();
            D3D_SAFE_RELEASE(mTexture2D);
            return false;
        }

#ifdef D3D_DEBUGGING
        // set debug name
        mDSV->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(textureName.length()),
                             textureName.c_str());
#endif // D3D_DEBUGGING
    }

    mWidth = desc.width;
    mHeight = desc.height;
    mLayers = desc.layers;
    mMipmaps = desc.mipmaps;
    mSamples = desc.samplesNum;
    mTexelSize = GetElementFormatSize(desc.format) * desc.texelSize;
    mType = TextureType::Texture2D;
    return true;
}

bool Texture::InitTexture3D(const TextureDesc& desc)
{
    // TODO: fill
    UNUSED(desc);

    mType = TextureType::Texture3D;
    return true;
}

bool Texture::Init(const TextureDesc& desc)
{
    switch (desc.type)
    {
    case TextureType::Texture1D:
        return InitTexture1D(desc);
    case TextureType::Texture2D:
    case TextureType::TextureCube:
        return InitTexture2D(desc);
    case TextureType::Texture3D:
        return InitTexture3D(desc);
    }

    return false;
}

} // namespace Renderer
} // namespace NFE
