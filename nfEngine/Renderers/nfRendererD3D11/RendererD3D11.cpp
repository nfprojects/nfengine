/**
 * @file    RendererD3D11.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's lowlevel API
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

std::unique_ptr<Device> gDevice;

void Log(const char* str)
{
    OutputDebugStringA(str);
    OutputDebugStringA("\n");
}

IDevice* Init()
{
    if (gDevice == nullptr)
    {
        gDevice.reset(new Device);
    }

    return gDevice.get();
}

void Release()
{
    gDevice.reset();
}

// TODO: this could be moved to RendererDriver.h
int GetElementFormatSize(ElementFormat format)
{
    switch (format)
    {
        case ElementFormat::Float_32:
        case ElementFormat::Int_32:
        case ElementFormat::Uint_32:
            return 4;
        case ElementFormat::Float_16:
        case ElementFormat::Int_16:
        case ElementFormat::Uint_16:
        case ElementFormat::Int_16_norm:
        case ElementFormat::Uint_16_norm:
            return 2;
        case ElementFormat::Int_8:
        case ElementFormat::Uint_8:
        case ElementFormat::Int_8_norm:
        case ElementFormat::Uint_8_norm:
            return 1;
    };

    return 0;
}

DXGI_FORMAT TranslateElementFormat(ElementFormat format, int size)
{
    switch (format)
    {
        case ElementFormat::Float_32:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R32_FLOAT;
                case 2:
                    return DXGI_FORMAT_R32G32_FLOAT;
                case 3:
                    return DXGI_FORMAT_R32G32B32_FLOAT;
                case 4:
                    return DXGI_FORMAT_R32G32B32A32_FLOAT;
            };
        case ElementFormat::Int_32:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R32_SINT;
                case 2:
                    return DXGI_FORMAT_R32G32_SINT;
                case 3:
                    return DXGI_FORMAT_R32G32B32_SINT;
                case 4:
                    return DXGI_FORMAT_R32G32B32A32_SINT;
            };
        case ElementFormat::Uint_32:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R32_UINT;
                case 2:
                    return DXGI_FORMAT_R32G32_UINT;
                case 3:
                    return DXGI_FORMAT_R32G32B32_UINT;
                case 4:
                    return DXGI_FORMAT_R32G32B32A32_UINT;
            };
        case ElementFormat::Float_16:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R16_FLOAT;
                case 2:
                    return DXGI_FORMAT_R16G16_FLOAT;
                case 4:
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;
            };
        case ElementFormat::Int_16:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R16_SINT;
                case 2:
                    return DXGI_FORMAT_R16G16_SINT;
                case 4:
                    return DXGI_FORMAT_R16G16B16A16_SINT;
            };
        case ElementFormat::Uint_16:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R16_UINT;
                case 2:
                    return DXGI_FORMAT_R16G16_UINT;
                case 4:
                    return DXGI_FORMAT_R16G16B16A16_UINT;
            };
        case ElementFormat::Int_16_norm:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R16_SNORM;
                case 2:
                    return DXGI_FORMAT_R16G16_SNORM;
                case 4:
                    return DXGI_FORMAT_R16G16B16A16_SNORM;
            };
        case ElementFormat::Uint_16_norm:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R16_UNORM;
                case 2:
                    return DXGI_FORMAT_R16G16_UNORM;
                case 4:
                    return DXGI_FORMAT_R16G16B16A16_UNORM;
            };
        case ElementFormat::Int_8:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R8_SINT;
                case 2:
                    return DXGI_FORMAT_R8G8_SINT;
                case 4:
                    return DXGI_FORMAT_R8G8B8A8_SINT;
            };
        case ElementFormat::Uint_8:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R8_UINT;
                case 2:
                    return DXGI_FORMAT_R8G8_UINT;
                case 4:
                    return DXGI_FORMAT_R8G8B8A8_UINT;
            };
        case ElementFormat::Int_8_norm:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R8_SNORM;
                case 2:
                    return DXGI_FORMAT_R8G8_SNORM;
                case 4:
                    return DXGI_FORMAT_R8G8B8A8_SNORM;
            };
        case ElementFormat::Uint_8_norm:
            switch (size)
            {
                case 1:
                    return DXGI_FORMAT_R8_UNORM;
                case 2:
                    return DXGI_FORMAT_R8G8_UNORM;
                case 4:
                    return DXGI_FORMAT_R8G8B8A8_UNORM;
            };
    };

    return DXGI_FORMAT_UNKNOWN;
}

D3D11_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func)
{
    switch (func)
    {
        case CompareFunc::Never:
            return D3D11_COMPARISON_NEVER;
        case CompareFunc::Less:
            return D3D11_COMPARISON_LESS;
        case CompareFunc::LessEqual:
            return D3D11_COMPARISON_LESS_EQUAL;
        case CompareFunc::Equal:
            return D3D11_COMPARISON_EQUAL;
        case CompareFunc::EqualGrater:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case CompareFunc::Grater:
            return D3D11_COMPARISON_GREATER;
        case CompareFunc::NotEqual:
            return D3D11_COMPARISON_NOT_EQUAL;
        case CompareFunc::Always:
            return D3D11_COMPARISON_ALWAYS;
    };

    return D3D11_COMPARISON_NEVER;
}

D3D11_TEXTURE_ADDRESS_MODE TranslateTextureWrapMode(TextureWrapMode mode)
{
    switch (mode)
    {
    case TextureWrapMode::Repeat:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    case TextureWrapMode::Clamp:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case TextureWrapMode::Mirror:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    case TextureWrapMode::Border:
        return D3D11_TEXTURE_ADDRESS_BORDER;
    };

    return D3D11_TEXTURE_ADDRESS_WRAP;
}

} // namespace Renderer
} // namespace NFE
