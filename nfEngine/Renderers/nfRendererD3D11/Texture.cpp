/**
 * @file    Texture.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's texture
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

Texture::Texture()
{
    mTextureGeneric = nullptr;
    type = TextureType::Unknown;
}

Texture::~Texture()
{
    switch (type)
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
    return true;
}

bool Texture::InitTexture2D(const TextureDesc& desc)
{
    HRESULT hr;
    D3D11_TEXTURE2D_DESC td;
    
    td.Width = desc.width;
    td.Height = desc.height;
    td.ArraySize = desc.layers;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE; // TODO
    td.Usage = D3D11_USAGE_IMMUTABLE; // TODO
    td.CPUAccessFlags = 0; // TODO: support for dynamic textures
    td.Format = TranslateElementFormat(desc.format, desc.texelSize);
    td.MipLevels = desc.mipmaps;
    td.MiscFlags = 0;

    if (desc.type == TextureType::TextureCube)
        td.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

    // TODO: multisampling
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;

    if (desc.dataDesc)
    {
        D3D11_SUBRESOURCE_DATA initialData[MAX_MIPMAP_LEVELS];
        for (int i = 0; i < desc.mipmaps; ++i)
        {
            initialData[i].pSysMem = desc.dataDesc[i].data;
            initialData[i].SysMemPitch = desc.dataDesc[i].lineSize;
            initialData[i].SysMemSlicePitch = desc.dataDesc[i].sliceSize;
        }

        hr = gDevice->Get()->CreateTexture2D(&td, initialData, &mTexture2D);
    }
    else
        hr = gDevice->Get()->CreateTexture2D(&td, NULL, &mTexture2D);

    if (FAILED(hr))
    {
        Log("Creation of 2D texture object failed");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
    ZeroMemory(&srvd, sizeof(srvd));
    srvd.Format = td.Format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = desc.mipmaps;
    srvd.Texture2D.MostDetailedMip = 0;
    hr = gDevice->Get()->CreateShaderResourceView(mTexture2D, &srvd, &mSRV);
    if (FAILED(hr))
    {
        Log("Creation of SRV for 2D texture failed");
        return false;
    }

    return true;
}

bool Texture::InitTexture3D(const TextureDesc& desc)
{
    // TODO: fill
    return true;
}

bool Texture::Init(const TextureDesc& desc)
{
    // TODO: check params

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
