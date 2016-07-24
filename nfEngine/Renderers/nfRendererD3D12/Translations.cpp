/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of nfEngine to Direct3D 12 translations functions.
 */

#include "PCH.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

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
    case ElementFormat::BC1:
        return DXGI_FORMAT_BC1_UNORM;
    case ElementFormat::BC2:
        return DXGI_FORMAT_BC2_UNORM;
    case ElementFormat::BC3:
        return DXGI_FORMAT_BC3_UNORM;
    case ElementFormat::BC4:
        return DXGI_FORMAT_BC4_UNORM;
    case ElementFormat::BC4_signed:
        return DXGI_FORMAT_BC4_SNORM;
    case ElementFormat::BC5:
        return DXGI_FORMAT_BC5_UNORM;
    case ElementFormat::BC5_signed:
        return DXGI_FORMAT_BC5_SNORM;
    case ElementFormat::BC6H:
        return DXGI_FORMAT_BC6H_UF16;
    case ElementFormat::BC6H_signed:
        return DXGI_FORMAT_BC6H_SF16;
    case ElementFormat::BC7:
        return DXGI_FORMAT_BC7_UNORM;
    };

    return DXGI_FORMAT_UNKNOWN;
}

D3D12_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func)
{
    switch (func)
    {
    case CompareFunc::Never:
        return D3D12_COMPARISON_FUNC_NEVER;
    case CompareFunc::Less:
        return D3D12_COMPARISON_FUNC_LESS;
    case CompareFunc::LessEqual:
        return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case CompareFunc::Equal:
        return D3D12_COMPARISON_FUNC_EQUAL;
    case CompareFunc::EqualGreater:
        return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case CompareFunc::Greater:
        return D3D12_COMPARISON_FUNC_GREATER;
    case CompareFunc::NotEqual:
        return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case CompareFunc::Pass:
        return D3D12_COMPARISON_FUNC_ALWAYS;
    };

    return D3D12_COMPARISON_FUNC_NEVER;
}

D3D12_BLEND TranslateBlendFunc(BlendFunc func)
{
    switch (func)
    {
    case BlendFunc::Zero:
        return D3D12_BLEND_ZERO;
    case BlendFunc::One:
        return D3D12_BLEND_ONE;

    case BlendFunc::SrcColor:
        return D3D12_BLEND_SRC_COLOR;
    case BlendFunc::DestColor:
        return D3D12_BLEND_DEST_COLOR;
    case BlendFunc::SrcAlpha:
        return D3D12_BLEND_SRC_ALPHA;
    case BlendFunc::DestAlpha:
        return D3D12_BLEND_DEST_ALPHA;

    case BlendFunc::OneMinusSrcColor:
        return D3D12_BLEND_INV_SRC_COLOR;
    case BlendFunc::OneMinusDestColor:
        return D3D12_BLEND_INV_DEST_COLOR;
    case BlendFunc::OneMinusSrcAlpha:
        return D3D12_BLEND_INV_SRC_ALPHA;
    case BlendFunc::OneMinusDestAlpha:
        return D3D12_BLEND_INV_DEST_ALPHA;
    };

    return D3D12_BLEND_ZERO;
}

D3D12_BLEND_OP TranslateBlendOp(BlendOp op)
{
    switch (op)
    {
    case BlendOp::Add:
        return D3D12_BLEND_OP_ADD;
    case BlendOp::Subtract:
        return D3D12_BLEND_OP_SUBTRACT;
    case BlendOp::RevSubtract:
        return D3D12_BLEND_OP_REV_SUBTRACT;
    case BlendOp::Min:
        return D3D12_BLEND_OP_MIN;
    case BlendOp::Max:
        return D3D12_BLEND_OP_MAX;
    };

    return D3D12_BLEND_OP_ADD;
}

D3D12_STENCIL_OP TranslateStencilOp(StencilOp op)
{
    switch (op)
    {
    case StencilOp::Keep:
        return D3D12_STENCIL_OP_KEEP;
    case StencilOp::Zero:
        return D3D12_STENCIL_OP_ZERO;
    case StencilOp::Replace:
        return D3D12_STENCIL_OP_REPLACE;
    case StencilOp::Increment:
        return D3D12_STENCIL_OP_INCR_SAT;
    case StencilOp::IncrementWrap:
        return D3D12_STENCIL_OP_INCR;
    case StencilOp::Decrement:
        return D3D12_STENCIL_OP_DECR_SAT;
    case StencilOp::DecrementWrap:
        return D3D12_STENCIL_OP_DECR;
    case StencilOp::Invert:
        return D3D12_STENCIL_OP_INVERT;
    };

    return D3D12_STENCIL_OP_KEEP;
}

D3D12_PRIMITIVE_TOPOLOGY TranslatePrimitiveType(PrimitiveType type, uint32 controlPoints)
{
    switch (type)
    {
    case PrimitiveType::Points:
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case PrimitiveType::Lines:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case PrimitiveType::LinesStrip:
        return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case PrimitiveType::Triangles:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PrimitiveType::TrianglesStrip:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case PrimitiveType::Patch:
        // hope that enum values are contiguous...
        return static_cast<D3D12_PRIMITIVE_TOPOLOGY>(
            static_cast<uint32>(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) +
            (controlPoints - 1));
    };

    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE TranslatePrimitiveTopologyType(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::Points:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case PrimitiveType::Lines:
    case PrimitiveType::LinesStrip:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case PrimitiveType::Triangles:
    case PrimitiveType::TrianglesStrip:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case PrimitiveType::Patch:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    }

    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

D3D12_FILTER TranslateFilterType(TextureMinFilter minFilter, TextureMagFilter magFilter,
                                 bool compare, bool anisotropic)
{
    if (!compare)
    {
        if (magFilter == TextureMagFilter::Nearest)
        {
            switch (minFilter)
            {
            case TextureMinFilter::NearestMipmapNearest:
                return D3D12_FILTER_MIN_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
        }
        else
        {
            switch (minFilter)
            {
            case TextureMinFilter::NearestMipmapNearest:
                return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                if (anisotropic)
                    return D3D12_FILTER_ANISOTROPIC;
                else
                    return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
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
                return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
        }
        else
        {
            switch (minFilter)
            {
            case TextureMinFilter::NearestMipmapNearest:
                return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapNearest:
                return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
            case TextureMinFilter::NearestMipmapLinear:
                return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            case TextureMinFilter::LinearMipmapLinear:
                if (anisotropic)
                    return D3D12_FILTER_COMPARISON_ANISOTROPIC;
                else
                    return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            }
        }
    }

    return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

D3D12_TEXTURE_ADDRESS_MODE TranslateTextureAddressMode(TextureWrapMode mode)
{
    switch (mode)
    {
    case TextureWrapMode::Repeat:
        return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case TextureWrapMode::Clamp:
        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case TextureWrapMode::Mirror:
        return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case TextureWrapMode::Border:
        return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    };

    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

bool TranslateShaderVisibility(ShaderType shaderType, D3D12_SHADER_VISIBILITY& visibility)
{
    switch (shaderType)
    {
    case ShaderType::Vertex:
        visibility = D3D12_SHADER_VISIBILITY_VERTEX;
        return true;
    case ShaderType::Hull:
        visibility = D3D12_SHADER_VISIBILITY_HULL;
        return true;
    case ShaderType::Domain:
        visibility = D3D12_SHADER_VISIBILITY_DOMAIN;
        return true;
    case ShaderType::Geometry:
        visibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
        return true;
    case ShaderType::Pixel:
        visibility = D3D12_SHADER_VISIBILITY_PIXEL;
        return true;
    case ShaderType::All:
        visibility = D3D12_SHADER_VISIBILITY_ALL;
        return true;
    }

    return false;
}

} // namespace Renderer
} // namespace NFE
