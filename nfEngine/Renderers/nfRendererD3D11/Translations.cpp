/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of nfEngine to Direct3D 11 translations functions.
 */

#include "PCH.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

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

D3D11_PRIMITIVE_TOPOLOGY TranslatePrimitiveType(PrimitiveType type)
{
    switch (type)
    {
        case PrimitiveType::Points:
            return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveType::Lines:
            return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveType::LinesStrip:
            return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveType::Triangles:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveType::TrianglesStrip:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    };

    return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
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

bool TranslateDepthBufferTypes(DepthBufferFormat inFormat, DXGI_FORMAT& resFormat,
                               DXGI_FORMAT& srvFormat, DXGI_FORMAT& dsvFormat)
{
    switch (inFormat)
    {
    case DepthBufferFormat::Depth16:
        resFormat = DXGI_FORMAT_R16_TYPELESS;
        srvFormat = DXGI_FORMAT_R16_UNORM;
        dsvFormat = DXGI_FORMAT_D16_UNORM;
        return true;
    case DepthBufferFormat::Depth24_Stencil8:
        resFormat = DXGI_FORMAT_R24G8_TYPELESS;
        srvFormat = DXGI_FORMAT_R16_UNORM;
        dsvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        return true;
    case DepthBufferFormat::Depth32:
        resFormat = DXGI_FORMAT_R32_TYPELESS;
        srvFormat = DXGI_FORMAT_R32_FLOAT;
        dsvFormat = DXGI_FORMAT_D32_FLOAT;
        return true;
    }

    return false;
}

D3D11_FILTER TranslateFilterType(TextureMinFilter minFilter, TextureMagFilter magFilter,
                                 bool compare, bool anisotropic)
{
    if (!compare)
    {
        if (magFilter == TextureMagFilter::Nearest)
        {
            switch (minFilter)
            {
            case TextureMinFilter::NearestMipmapNearest:
                return D3D11_FILTER_MIN_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
        }
        else
        {
            switch (minFilter)
            {
            case TextureMinFilter::NearestMipmapNearest:
                return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                if (anisotropic)
                    return D3D11_FILTER_ANISOTROPIC;
                else
                    return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            }
        }
    }
    else // compare == true
    {
        if (magFilter == TextureMagFilter::Nearest)
        {
            switch (minFilter)
            {
            case TextureMinFilter::NearestMipmapNearest:
                return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
        }
        else
        {
            switch (minFilter)
            {
            case TextureMinFilter::NearestMipmapNearest:
                return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                if (anisotropic)
                    return D3D11_FILTER_COMPARISON_ANISOTROPIC;
                else
                    return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            }
        }
    }

    return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

D3D11_BLEND TranslateBlendFunc(BlendFunc func)
{
    switch (func)
    {
    case BlendFunc::Zero:
        return D3D11_BLEND_ZERO;
    case BlendFunc::One:
        return D3D11_BLEND_ONE;

    case BlendFunc::SrcColor:
        return D3D11_BLEND_SRC_COLOR;
    case BlendFunc::DestColor:
        return D3D11_BLEND_DEST_COLOR;
    case BlendFunc::SrcAlpha:
        return D3D11_BLEND_SRC_ALPHA;
    case BlendFunc::DestAlpha:
        return D3D11_BLEND_DEST_ALPHA;

    case BlendFunc::OneMinusSrcColor:
        return D3D11_BLEND_INV_SRC_COLOR;
    case BlendFunc::OneMinusDestColor:
        return D3D11_BLEND_INV_DEST_COLOR;
    case BlendFunc::OneMinusSrcAlpha:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case BlendFunc::OneMinusDestAlpha:
        return D3D11_BLEND_INV_DEST_ALPHA;
    };

    return D3D11_BLEND_ZERO;
}

D3D11_BLEND_OP TranslateBlendOp(BlendOp op)
{
    switch (op)
    {
    case BlendOp::Add:
        return D3D11_BLEND_OP_ADD;
    case BlendOp::Subtract:
        return D3D11_BLEND_OP_SUBTRACT;
    case BlendOp::RevSubtract:
        return D3D11_BLEND_OP_REV_SUBTRACT;
    case BlendOp::Min:
        return D3D11_BLEND_OP_MIN;
    case BlendOp::Max:
        return D3D11_BLEND_OP_MAX;
    };

    return D3D11_BLEND_OP_ADD;
}

} // namespace Renderer
} // namespace NFE
