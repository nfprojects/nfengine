/**
 * @file
 * @author  Witek902
 */

#include "PCH.hpp"
#include "Types.hpp"


NFE_BEGIN_DEFINE_ENUM(NFE::Renderer::Format)
    NFE_ENUM_OPTION(Unknown)
    NFE_ENUM_OPTION(R8_U_Int)
    NFE_ENUM_OPTION(R8_S_Int)
    NFE_ENUM_OPTION(R8_U_Norm)
    NFE_ENUM_OPTION(R8_S_Norm)
    NFE_ENUM_OPTION(R16_U_Int)
    NFE_ENUM_OPTION(R16_S_Int)
    NFE_ENUM_OPTION(R16_U_Norm)
    NFE_ENUM_OPTION(R16_S_Norm)
    NFE_ENUM_OPTION(R16_Float)
    NFE_ENUM_OPTION(R32_U_Int)
    NFE_ENUM_OPTION(R32_S_Int)
    NFE_ENUM_OPTION(R32_Float)
    NFE_ENUM_OPTION(R8G8_U_Int)
    NFE_ENUM_OPTION(R8G8_S_Int)
    NFE_ENUM_OPTION(R8G8_U_Norm)
    NFE_ENUM_OPTION(R8G8_S_Norm)
    NFE_ENUM_OPTION(R16G16_U_Int)
    NFE_ENUM_OPTION(R16G16_S_Int)
    NFE_ENUM_OPTION(R16G16_U_Norm)
    NFE_ENUM_OPTION(R16G16_S_Norm)
    NFE_ENUM_OPTION(R16G16_Float)
    NFE_ENUM_OPTION(R32G32_U_Int)
    NFE_ENUM_OPTION(R32G32_S_Int)
    NFE_ENUM_OPTION(R32G32_Float)
    NFE_ENUM_OPTION(R32G32B32_U_Int)
    NFE_ENUM_OPTION(R32G32B32_S_Int)
    NFE_ENUM_OPTION(R32G32B32_Float)
    NFE_ENUM_OPTION(R8G8B8A8_U_Int)
    NFE_ENUM_OPTION(R8G8B8A8_S_Int)
    NFE_ENUM_OPTION(R8G8B8A8_U_Norm)
    NFE_ENUM_OPTION(R8G8B8A8_U_Norm_sRGB)
    NFE_ENUM_OPTION(R8G8B8A8_S_Norm)
    NFE_ENUM_OPTION(B8G8R8A8_U_Norm)
    NFE_ENUM_OPTION(B8G8R8A8_U_Norm_sRGB)
    NFE_ENUM_OPTION(R16G16B16A16_U_Int)
    NFE_ENUM_OPTION(R16G16B16A16_S_Int)
    NFE_ENUM_OPTION(R16G16B16A16_U_Norm)
    NFE_ENUM_OPTION(R16G16B16A16_S_Norm)
    NFE_ENUM_OPTION(R16G16B16A16_Float)
    NFE_ENUM_OPTION(R32G32B32A32_U_Int)
    NFE_ENUM_OPTION(R32G32B32A32_S_Int)
    NFE_ENUM_OPTION(R32G32B32A32_Float)
    NFE_ENUM_OPTION(B5G6R5_U_norm)
    NFE_ENUM_OPTION(B5G5R5A1_U_Norm)
    NFE_ENUM_OPTION(B4G4R4A4_U_Norm)
    NFE_ENUM_OPTION(R11G11B10_Float)
    NFE_ENUM_OPTION(R10G10B10A2_U_Norm)
    NFE_ENUM_OPTION(R10G10B10A2_U_Int)
    NFE_ENUM_OPTION(R9G9B9E5_Float)
    NFE_ENUM_OPTION(BC1_U_Norm)
    NFE_ENUM_OPTION(BC1_U_Norm_sRGB)
    NFE_ENUM_OPTION(BC2_U_Norm)
    NFE_ENUM_OPTION(BC2_U_Norm_sRGB)
    NFE_ENUM_OPTION(BC3_U_Norm)
    NFE_ENUM_OPTION(BC3_U_Norm_sRGB)
    NFE_ENUM_OPTION(BC4_U_Norm)
    NFE_ENUM_OPTION(BC4_S_Norm)
    NFE_ENUM_OPTION(BC5_U_Norm)
    NFE_ENUM_OPTION(BC5_S_Norm)
    NFE_ENUM_OPTION(BC6H_U_Float)
    NFE_ENUM_OPTION(BC6H_S_Float)
    NFE_ENUM_OPTION(BC7_U_Norm)
    NFE_ENUM_OPTION(BC7_U_Norm_sRGB)
NFE_END_DEFINE_ENUM()
static_assert(64 == (int)NFE::Renderer::Format::Max, "Format list changed, update the list above");

namespace NFE {
namespace Renderer {


uint32 GetElementFormatChannels(const Format format)
{
    switch (format)
    {
    case Format::R8_U_Int:
    case Format::R8_S_Int:
    case Format::R8_U_Norm:
    case Format::R8_S_Norm:
    case Format::R16_U_Int:
    case Format::R16_S_Int:
    case Format::R16_U_Norm:
    case Format::R16_S_Norm:
    case Format::R16_Float:
    case Format::R32_U_Int:
    case Format::R32_S_Int:
    case Format::R32_Float:
        return 1;

    case Format::R8G8_U_Int:
    case Format::R8G8_S_Int:
    case Format::R8G8_U_Norm:
    case Format::R8G8_S_Norm:
    case Format::R16G16_U_Int:
    case Format::R16G16_S_Int:
    case Format::R16G16_U_Norm:
    case Format::R16G16_S_Norm:
    case Format::R16G16_Float:
    case Format::R32G32_U_Int:
    case Format::R32G32_S_Int:
    case Format::R32G32_Float:
        return 2;

    case Format::R32G32B32_U_Int:
    case Format::R32G32B32_S_Int:
    case Format::R32G32B32_Float:
    case Format::B5G6R5_U_norm:
    case Format::R11G11B10_Float:
    case Format::R9G9B9E5_Float:
        return 3;

    case Format::R8G8B8A8_U_Int:
    case Format::R8G8B8A8_S_Int:
    case Format::R8G8B8A8_U_Norm:
    case Format::R8G8B8A8_U_Norm_sRGB:
    case Format::R8G8B8A8_S_Norm:
    case Format::B8G8R8A8_U_Norm:
    case Format::B8G8R8A8_U_Norm_sRGB:
    case Format::R16G16B16A16_U_Int:
    case Format::R16G16B16A16_S_Int:
    case Format::R16G16B16A16_U_Norm:
    case Format::R16G16B16A16_S_Norm:
    case Format::R16G16B16A16_Float:
    case Format::R32G32B32A32_U_Int:
    case Format::R32G32B32A32_S_Int:
    case Format::R32G32B32A32_Float:
    case Format::B5G5R5A1_U_Norm:
    case Format::B4G4R4A4_U_Norm:
    case Format::R10G10B10A2_U_Norm:
    case Format::R10G10B10A2_U_Int:
        return 4;

    case Format::BC1_U_Norm:
    case Format::BC1_U_Norm_sRGB:
    case Format::BC2_U_Norm:
    case Format::BC2_U_Norm_sRGB:
    case Format::BC3_U_Norm:
    case Format::BC3_U_Norm_sRGB:
        return 4;

    case Format::BC4_U_Norm:
    case Format::BC4_S_Norm:
        return 1;

    case Format::BC5_U_Norm:
    case Format::BC5_S_Norm:
        return 2;

    case Format::BC6H_U_Float:
    case Format::BC6H_S_Float:
    case Format::BC7_U_Norm:
    case Format::BC7_U_Norm_sRGB:
        return 3;
    }

    static_assert(64 == (uint32)Format::Max, "Format list changed, update the switch above");

    return 0;
}


NFE_INLINE uint32 GetElementFormatSize(const Format format)
{
    switch (format)
    {
    case Format::R8_U_Int:
    case Format::R8_S_Int:
    case Format::R8_U_Norm:
    case Format::R8_S_Norm:
        return 1;
    case Format::R16_U_Int:
    case Format::R16_S_Int:
    case Format::R16_U_Norm:
    case Format::R16_S_Norm:
    case Format::R16_Float:
        return 2;
    case Format::R32_U_Int:
    case Format::R32_S_Int:
    case Format::R32_Float:
        return 4;
    case Format::R8G8_U_Int:
    case Format::R8G8_S_Int:
    case Format::R8G8_U_Norm:
    case Format::R8G8_S_Norm:
        return 2 * 1;
    case Format::R16G16_U_Int:
    case Format::R16G16_S_Int:
    case Format::R16G16_U_Norm:
    case Format::R16G16_S_Norm:
    case Format::R16G16_Float:
        return 2 * 2;
    case Format::R32G32_U_Int:
    case Format::R32G32_S_Int:
    case Format::R32G32_Float:
        return 2 * 4;
    case Format::R32G32B32_U_Int:
    case Format::R32G32B32_S_Int:
    case Format::R32G32B32_Float:
        return 3 * 4;
    case Format::R8G8B8A8_U_Int:
    case Format::R8G8B8A8_S_Int:
    case Format::R8G8B8A8_U_Norm:
    case Format::R8G8B8A8_U_Norm_sRGB:
    case Format::R8G8B8A8_S_Norm:
    case Format::B8G8R8A8_U_Norm:
    case Format::B8G8R8A8_U_Norm_sRGB:
        return 4 * 1;
    case Format::R16G16B16A16_U_Int:
    case Format::R16G16B16A16_S_Int:
    case Format::R16G16B16A16_U_Norm:
    case Format::R16G16B16A16_S_Norm:
    case Format::R16G16B16A16_Float:
        return 4 * 2;
    case Format::R32G32B32A32_U_Int:
    case Format::R32G32B32A32_S_Int:
    case Format::R32G32B32A32_Float:
        return 4 * 4;
    case Format::B5G6R5_U_norm:
    case Format::B5G5R5A1_U_Norm:
    case Format::B4G4R4A4_U_Norm:
        return 2;
    case Format::R11G11B10_Float:
    case Format::R10G10B10A2_U_Norm:
    case Format::R10G10B10A2_U_Int:
    case Format::R9G9B9E5_Float:
        return 4;
    case Format::BC1_U_Norm:
    case Format::BC1_U_Norm_sRGB:
    case Format::BC4_U_Norm:
    case Format::BC4_S_Norm:
        return 8;
    case Format::BC2_U_Norm:
    case Format::BC2_U_Norm_sRGB:
    case Format::BC3_U_Norm:
    case Format::BC3_U_Norm_sRGB:
    case Format::BC5_U_Norm:
    case Format::BC5_S_Norm:
    case Format::BC6H_U_Float:
    case Format::BC6H_S_Float:
    case Format::BC7_U_Norm:
    case Format::BC7_U_Norm_sRGB:
        return 16;
    }

    static_assert(64 == (uint32)Format::Max, "Format list changed, update the switch above");

    return 0;
}


const char* GetElementFormatName(const Format format)
{
    const RTTI::EnumType* type = RTTI::GetType<Format>();
    const Common::StringView formatName = type->FindOptionByValue((uint64)format);

    NFE_ASSERT(!formatName.Empty(), "Unknown format: %u", (uint32)format);
    NFE_ASSERT(formatName.IsNullTerminated(), "Enum option string should be null terminated");

    return formatName.Data();
}


} // namespace Renderer
} // namespace NFE
