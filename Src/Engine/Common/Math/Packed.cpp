#include "PCH.hpp"
#include "Packed.hpp"
#include "Vec4i.hpp"

namespace NFE {
namespace Math {

static_assert(sizeof(PackedUnitVector3) == 4, "Invalid size of PackedUnitVector3");
static_assert(sizeof(PackedColorRgbHdr) == 8, "Invalid size of PackedColorRgbHdr");
static_assert(sizeof(PackedUFloat3_9_9_9_5) == 4, "Invalid size of PackedUFloat3_9_9_9_5");
static_assert(sizeof(PackedUFloat3_11_11_10) == 4, "Invalid size of PackedUFloat3_11_11_10");
static_assert(sizeof(Packed_5_6_5) == 2, "Invalid size of Packed_5_6_5");
static_assert(sizeof(Packed_4_4_4_4) == 2, "Invalid size of Packed_4_4_4_4");
static_assert(sizeof(Packed_10_10_10_2) == 4, "Invalid size of Packed_10_10_10_2");

const PackedUnitVector3 PackedUnitVector3::FromVector(const Vec4f& input)
{
    const Vec4f vAbs = Vec4f::Abs(input);
    Vec4f n = input / (vAbs.x + vAbs.y + vAbs.z);

    if (input.z < 0.0f)
    {
        //(1.0 - abs(v.yx))* (v.xy >= 0.0 ? 1.0 : -1.0);
        n = n.Swizzle<1, 0, 1, 0>();
        n = (Vec4f(1.0f) - Vec4f::Abs(n)).ChangeSign(input < Vec4f::Zero());
    }

    const Vec4i i = Vec4i::Convert(n * Scale);

    PackedUnitVector3 result;
    result.u = static_cast<int16>(i.x);
    result.v = static_cast<int16>(i.y);

    return result;
}

const PackedColorRgbHdr PackedColorRgbHdr::FromVector(const Vec4f& color)
{
    NFE_ASSERT((color >= Vec4f::Zero()).All(), "Color cannot be negative");

    Vec4f ycocg = color.SplatX() * Vec4f(0.25f, 0.5f * ChromaScale, -0.25f * ChromaScale);
    ycocg = Vec4f::MulAndAdd(color.SplatY(), Vec4f(0.5f, 0.0f, 0.5f * ChromaScale), ycocg);
    ycocg = Vec4f::MulAndAdd(color.SplatZ(), Vec4f(0.25f, -0.5f * ChromaScale, -0.25f * ChromaScale), ycocg);

    PackedColorRgbHdr result;

    result.y = ycocg.x;

    if (ycocg.x > 0.0f)
    {
        ycocg /= result.y;
    }

    const Vec4i i = Vec4i::Convert(ycocg);
    result.co = static_cast<int16>(i.y);
    result.cg = static_cast<int16>(i.z);

    return result;
}

const PackedUFloat3_9_9_9_5 PackedUFloat3_9_9_9_5::FromVector(const Vec4f& vec)
{
    // based on DirectXMath

    constexpr float maxf9 = float(0x1FF << 7);
    constexpr float minf9 = float(1.0f / (1 << 16));

    PackedUFloat3_9_9_9_5 result;

    const float x = (vec.x >= 0.f) ? ((vec.x > maxf9) ? maxf9 : vec.x) : 0.f;
    const float y = (vec.y >= 0.f) ? ((vec.y > maxf9) ? maxf9 : vec.y) : 0.f;
    const float z = (vec.z >= 0.f) ? ((vec.z > maxf9) ? maxf9 : vec.z) : 0.f;

    const float max_xy = (x > y) ? x : y;
    const float max_xyz = (max_xy > z) ? max_xy : z;

    const float maxColor = (max_xyz > minf9) ? max_xyz : minf9;

    Common::FundamentalTypesUnion fi;
    fi.f = maxColor;
    fi.i32 += 0x00004000;

    uint32 exp = static_cast<uint32>(fi.i32) >> 23;
    result.e = exp - 0x6f;

    fi.i32 = static_cast<int32_t>(0x83000000 - (exp << 23));
    const float scale = fi.f;

    result.mx = static_cast<uint32>(roundf(x * scale));
    result.my = static_cast<uint32>(roundf(y * scale));
    result.mz = static_cast<uint32>(roundf(z * scale));

    return result;
}

} // namespace Math
} // namespace NFE
