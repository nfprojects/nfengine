#pragma once

#include "Vec4f.hpp"

#include <emmintrin.h>

namespace NFE {
namespace Math {

VecBool4f::VecBool4f(bool x, bool y, bool z, bool w)
{
    v = _mm_castsi128_ps(_mm_set_epi32(w ? 0xFFFFFFFF : 0, z ? 0xFFFFFFFF : 0, y ? 0xFFFFFFFF : 0, x ? 0xFFFFFFFF : 0));
}

template<uint32 index>
bool VecBool4f::Get() const
{
    static_assert(index < 4, "Invalid index");
    return _mm_extract_ps(v, index) != 0;
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const VecBool4f VecBool4f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    if (ix == 0 && iy == 0 && iz == 1 && iw == 1)
    {
        return _mm_unpacklo_ps(v, v);
    }
    else if (ix == 2 && iy == 2 && iz == 3 && iw == 3)
    {
        return _mm_unpackhi_ps(v, v);
    }
    else if (ix == 0 && iy == 1 && iz == 0 && iw == 1)
    {
        return _mm_movelh_ps(v, v);
    }
    else if (ix == 2 && iy == 3 && iz == 2 && iw == 3)
    {
        return _mm_movehl_ps(v, v);
    }
    else if (ix == 0 && iy == 0 && iz == 2 && iw == 2)
    {
        return _mm_moveldup_ps(v);
    }
    else if (ix == 1 && iy == 1 && iz == 3 && iw == 3)
    {
        return _mm_movehdup_ps(v);
    }

    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

// combine into 4-bit mask
int VecBool4f::GetMask() const
{
    return _mm_movemask_ps(v);
}

bool VecBool4f::All() const
{
    return _mm_movemask_ps(v) == 0xF;
}

bool VecBool4f::None() const
{
    return _mm_movemask_ps(v) == 0;
}

bool VecBool4f::Any() const
{
    return _mm_movemask_ps(v) != 0;
}

bool VecBool4f::All3() const
{
    return (_mm_movemask_ps(v) & 0x7) == 0x7;
}

bool VecBool4f::None3() const
{
    return (_mm_movemask_ps(v) & 0x7) == 0;
}

bool VecBool4f::Any3() const
{
    return (_mm_movemask_ps(v) & 0x7) != 0;
}

const VecBool4f VecBool4f::operator & (const VecBool4f rhs) const
{
    return _mm_and_ps(v, rhs.v);
}

const VecBool4f VecBool4f::operator | (const VecBool4f rhs) const
{
    return _mm_or_ps(v, rhs.v);
}

const VecBool4f VecBool4f::operator ^ (const VecBool4f rhs) const
{
    return _mm_xor_ps(v, rhs.v);
}

bool VecBool4f::operator == (const VecBool4f rhs) const
{
    return GetMask() == rhs.GetMask();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec4f Vec4f::Zero()
{
    return _mm_setzero_ps();
}

#ifdef _DEBUG
Vec4f::Vec4f()
    : v(_mm_set1_ps(std::numeric_limits<float>::signaling_NaN()))
{}
#else
Vec4f::Vec4f() = default;
#endif // _DEBUG

Vec4f::Vec4f(const Vec4f& other)
    : v(other.v)
{}

Vec4f::Vec4f(const __m128& src)
    : v(src)
{ }

Vec4f::Vec4f(const float s)
    : v(_mm_set1_ps(s))
{}

Vec4f::Vec4f(const int32 i)
    : v(_mm_castsi128_ps(_mm_set1_epi32(i)))
{}

Vec4f::Vec4f(const uint32 u)
    : v(_mm_castsi128_ps(_mm_set1_epi32(u)))
{}

Vec4f::Vec4f(const float x, const float y, const float z, const float w)
    : v(_mm_set_ps(w, z, y, x))
{}

Vec4f::Vec4f(const int32 x, const int32 y, const int32 z, const int32 w)
    : v(_mm_castsi128_ps(_mm_set_epi32(w, z, y, x)))
{}

Vec4f::Vec4f(const uint32 x, const uint32 y, const uint32 z, const uint32 w)
    : v(_mm_castsi128_ps(_mm_set_epi32(w, z, y, x)))
{}

Vec4f::Vec4f(const float* src)
    : v(_mm_loadu_ps(src))
{}

Vec4f::Vec4f(const Vec2f& src)
{
    __m128 vx = _mm_load_ss(&src.x);
    __m128 vy = _mm_load_ss(&src.y);
    v = _mm_unpacklo_ps(vx, vy);
}

Vec4f::Vec4f(const Vec3f& src)
{
    __m128 vx = _mm_load_ss(&src.x);
    __m128 vy = _mm_load_ss(&src.y);
    __m128 vz = _mm_load_ss(&src.z);
    __m128 vxy = _mm_unpacklo_ps(vx, vy);
    v = _mm_movelh_ps(vxy, vz);
}

Vec4f::Vec4f(const Vec4fU& src)
    : v(_mm_loadu_ps(&src.x))
{}

Vec4f& Vec4f::operator = (const Vec4f& other)
{
    v = other.v;
    return *this;
}

const Vec4f Vec4f::FromInteger(int32 x)
{
    return _mm_cvtepi32_ps(_mm_set1_epi32(x));
}

const Vec4f Vec4f::FromIntegers(int32 x, int32 y, int32 z, int32 w)
{
    return _mm_cvtepi32_ps(_mm_set_epi32(w, z, y, x));
}

uint32 Vec4f::ToBGR() const
{
    const Vec4f scaled = (*this) * VECTOR_255;

    // convert to int and clamp to range
    __m128i vInt = _mm_cvttps_epi32(scaled);
    vInt = _mm_max_epi32(vInt, _mm_setzero_si128());
    vInt = _mm_min_epi32(vInt, _mm_set1_epi32(255));

    // extract RGB components:
    // in: 000000BB  000000GG  000000RR
    // out:                    00RRGGBB
    //const __m128i b = _mm_srli_si128(vInt, 8);
    //const __m128i g = _mm_srli_si128(vInt, 3);
    //const __m128i r = _mm_slli_epi32(vInt, 2 * 8);
    //const __m128i result = _mm_or_si128(r, _mm_or_si128(g, b));

    const __m128i shuffleMask = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8);
    const __m128i result = _mm_shuffle_epi8(vInt, shuffleMask);

    return _mm_extract_epi32(result, 0);
}

uint32 Vec4f::ToRGBA() const
{
    // Set <0 to 0
    __m128 vResult = _mm_max_ps(v, _mm_setzero_ps());
    // Set>1 to 1
    vResult = _mm_min_ps(vResult, VECTOR_ONE);
    // Convert to 0-255
    vResult = _mm_mul_ps(vResult, VECTOR_255);
    // Shuffle RGBA to ABGR
    vResult = _mm_shuffle_ps(vResult, vResult, _MM_SHUFFLE(3, 2, 1, 0));
    // Convert to int
    __m128i vInt = _mm_cvtps_epi32(vResult);
    // Mash to shorts
    vInt = _mm_packs_epi32(vInt, vInt);
    // Mash to bytes
    vInt = _mm_packus_epi16(vInt, vInt);
    // Return color
    return _mm_extract_epi32(vInt, 0);
}

const Half4 Vec4f::ToHalf4() const
{
    Half4 halfs;
#ifdef NFE_USE_FP16C
#if defined(NFE_ARCH_X64)
    halfs.packed = static_cast<uint64>(_mm_cvtsi128_si64(_mm_cvtps_ph(v, 0)));
#elif defined(NFE_ARCH_X86)
#endif
#else // NFE_USE_FP16C
    halfs.x = Half{ other.x };
    halfs.y = Half{ other.y };
    halfs.z = Half{ other.z };
    halfs.w = Half{ other.w };
#endif // NFE_USE_FP16C
    return halfs;
}

template<uint32 flipX, uint32 flipY, uint32 flipZ, uint32 flipW>
const Vec4f Vec4f::ChangeSign() const
{
    if (!(flipX || flipY || flipZ || flipW))
    {
        // no operation
        return *this;
    }

    // generate bit negation mask
    const Vec4f mask{ flipX ? 0x80000000 : 0, flipY ? 0x80000000 : 0, flipZ ? 0x80000000 : 0, flipW ? 0x80000000 : 0 };

    // flip sign bits
    return _mm_xor_ps(v, mask);
}

const Vec4f Vec4f::ChangeSign(const VecBool4f& flip) const
{
    return _mm_xor_ps(v, _mm_castsi128_ps(_mm_slli_epi32(flip, 31)));
}

template<uint32 maskX, uint32 maskY, uint32 maskZ, uint32 maskW>
NFE_FORCE_INLINE const Vec4f Vec4f::MakeMask()
{
    static_assert(!(maskX == 0 && maskY == 0 && maskZ == 0 && maskW == 0), "Useless mask");
    static_assert(!(maskX && maskY && maskZ && maskW), "Useless mask");

    // generate bit negation mask
    return Vec4f{ maskX ? 0xFFFFFFFF : 0, maskY ? 0xFFFFFFFF : 0, maskZ ? 0xFFFFFFFF : 0, maskW ? 0xFFFFFFFF : 0 };
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4f Vec4f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    if (ix == 0 && iy == 1 && iz == 2 && iw == 3)
    {
        return *this;
    }
    else if (ix == 0 && iy == 0 && iz == 1 && iw == 1)
    {
        return _mm_unpacklo_ps(v, v);
    }
    else if (ix == 2 && iy == 2 && iz == 3 && iw == 3)
    {
        return _mm_unpackhi_ps(v, v);
    }
    else if (ix == 0 && iy == 1 && iz == 0 && iw == 1)
    {
        return _mm_movelh_ps(v, v);
    }
    else if (ix == 2 && iy == 3 && iz == 2 && iw == 3)
    {
        return _mm_movehl_ps(v, v);
    }
    else if (ix == 0 && iy == 0 && iz == 2 && iw == 2)
    {
        return _mm_moveldup_ps(v);
    }
    else if (ix == 1 && iy == 1 && iz == 3 && iw == 3)
    {
        return _mm_movehdup_ps(v);
    }
#ifdef NFE_USE_AVX2
    else if (ix == 0 && iy == 0 && iz == 0 && iw == 0)
    {
        return _mm_broadcastss_ps(v);
    }
#endif // NFE_USE_AVX2

    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(iw, iz, iy, ix));
}

const Vec4f Vec4f::Swizzle(uint32 ix, uint32 iy, uint32 iz, uint32 iw) const
{
#ifdef NFE_USE_AVX
    return _mm_permutevar_ps(v, _mm_set_epi32(iw, iz, iy, ix));
#else
    return Vec4f{ f[ix], f[iy], f[iz], f[iw] };
#endif
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4f Vec4f::Shuffle(const Vec4f& a, const Vec4f& b)
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    if (ix == 0 && iy == 1 && iz == 0 && iw == 1)
    {
        return _mm_movelh_ps(a, b);
    }
    else if (ix == 2 && iy == 3 && iz == 2 && iw == 3)
    {
        return _mm_movehl_ps(b, a);
    }

    return _mm_shuffle_ps(a, b, ix | (iy << 2) | (iz << 4) | (iw << 6));
}

const Vec4f Vec4f::Select(const Vec4f& a, const Vec4f& b, const VecBool4f& sel)
{
    return _mm_blendv_ps(a, b, sel.v);
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vec4f Vec4f::Select(const Vec4f& a, const Vec4f& b)
{
    static_assert(selX <= 1, "Invalid X index");
    static_assert(selY <= 1, "Invalid Y index");
    static_assert(selZ <= 1, "Invalid Z index");
    static_assert(selW <= 1, "Invalid W index");

    return _mm_blend_ps(a, b, selX | (selY << 1) | (selZ << 2) | (selW << 3));
}

const Vec4f Vec4f::operator& (const Vec4f& b) const
{
    return _mm_and_ps(v, b);
}

const Vec4f Vec4f::operator| (const Vec4f& b) const
{
    return _mm_or_ps(v, b);
}

const Vec4f Vec4f::operator^ (const Vec4f& b) const
{
    return _mm_xor_ps(v, b);
}

Vec4f& Vec4f::operator&= (const Vec4f& b)
{
    v = _mm_and_ps(v, b);
    return *this;
}

Vec4f& Vec4f::operator|= (const Vec4f& b)
{
    v = _mm_or_ps(v, b);
    return *this;
}

Vec4f& Vec4f::operator^= (const Vec4f& b)
{
    v = _mm_xor_ps(v, b);
    return *this;
}

const Vec4f Vec4f::operator- () const
{
    return Vec4f::Zero() - (*this);
}

const Vec4f Vec4f::operator+ (const Vec4f& b) const
{
    return _mm_add_ps(v, b);
}

const Vec4f Vec4f::operator- (const Vec4f& b) const
{
    return _mm_sub_ps(v, b);
}

const Vec4f Vec4f::operator* (const Vec4f& b) const
{
    return _mm_mul_ps(v, b);
}

const Vec4f Vec4f::operator/ (const Vec4f& b) const
{
    return _mm_div_ps(v, b);
}

const Vec4f Vec4f::operator* (float b) const
{
    return _mm_mul_ps(v, _mm_set1_ps(b));
}

const Vec4f Vec4f::operator/ (float b) const
{
    return _mm_div_ps(v, _mm_set1_ps(b));
}

const Vec4f operator*(float a, const Vec4f& b)
{
    return _mm_mul_ps(b, _mm_set1_ps(a));
}


Vec4f& Vec4f::operator+= (const Vec4f& b)
{
    v = _mm_add_ps(v, b);
    return *this;
}

Vec4f& Vec4f::operator-= (const Vec4f& b)
{
    v = _mm_sub_ps(v, b);
    return *this;
}

Vec4f& Vec4f::operator*= (const Vec4f& b)
{
    v = _mm_mul_ps(v, b);
    return *this;
}

Vec4f& Vec4f::operator/= (const Vec4f& b)
{
    v = _mm_div_ps(v, b);
    return *this;
}

Vec4f& Vec4f::operator*= (float b)
{
    v = _mm_mul_ps(v, _mm_set1_ps(b));
    return *this;
}

Vec4f& Vec4f::operator/= (float b)
{
    v = _mm_div_ps(v, _mm_set1_ps(b));
    return *this;
}

const Vec4f Vec4f::Mod1(const Vec4f& x)
{
    return x - Vec4f::Floor(x);
}

const Vec4f Vec4f::MulAndAdd(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
#ifdef NFE_USE_FMA
    return _mm_fmadd_ps(a, b, c);
#else
    return a * b + c;
#endif
}

const Vec4f Vec4f::MulAndSub(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
#ifdef NFE_USE_FMA
    return _mm_fmsub_ps(a, b, c);
#else
    return a * b - c;
#endif
}

const Vec4f Vec4f::NegMulAndAdd(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
#ifdef NFE_USE_FMA
    return _mm_fnmadd_ps(a, b, c);
#else
    return -(a * b) + c;
#endif
}

const Vec4f Vec4f::NegMulAndSub(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
#ifdef NFE_USE_FMA
    return _mm_fnmsub_ps(a, b, c);
#else
    return -(a * b) - c;
#endif
}

const Vec4f Vec4f::Floor(const Vec4f& v)
{
    return _mm_floor_ps(v);
}

const Vec4f Vec4f::Sqrt(const Vec4f& V)
{
    return _mm_sqrt_ps(V);
}

const Vec4f Vec4f::Reciprocal(const Vec4f& V)
{
    return _mm_div_ps(VECTOR_ONE, V);
}

const Vec4f Vec4f::FastReciprocal(const Vec4f& v)
{
    const __m128 rcp = _mm_rcp_ps(v);
    const __m128 rcpSqr = _mm_mul_ps(rcp, rcp);
    const __m128 rcp2 = _mm_add_ps(rcp, rcp);
    return NegMulAndAdd(rcpSqr, v, rcp2);
}

const Vec4f Vec4f::Min(const Vec4f& a, const Vec4f& b)
{
    return _mm_min_ps(a, b);
}

const Vec4f Vec4f::Max(const Vec4f& a, const Vec4f& b)
{
    return _mm_max_ps(a, b);
}

const Vec4f Vec4f::Abs(const Vec4f& v)
{
    return _mm_and_ps(v, VECTOR_MASK_ABS);
}

uint32 Vec4f::GetSignMask() const
{
    return static_cast<uint32>(_mm_movemask_ps(v));
}

const Vec4f Vec4f::HorizontalMax() const
{
    __m128 temp;
    temp = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
    temp = _mm_max_ps(temp, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
    return temp;
}

const Vec4f Vec4f::HorizontalSum() const
{
    const __m128 temp = _mm_hadd_ps(v, v);
    return _mm_hadd_ps(temp, temp);
}

const VecBool4f Vec4f::operator == (const Vec4f& b) const
{
    return _mm_cmpeq_ps(v, b.v);
}

const VecBool4f Vec4f::operator < (const Vec4f& b) const
{
    return _mm_cmplt_ps(v, b.v);
}

const VecBool4f Vec4f::operator <= (const Vec4f& b) const
{
    return _mm_cmple_ps(v, b.v);
}

const VecBool4f Vec4f::operator > (const Vec4f& b) const
{
    return _mm_cmpgt_ps(v, b.v);
}

const VecBool4f Vec4f::operator >= (const Vec4f& b) const
{
    return _mm_cmpge_ps(v, b.v);
}

const VecBool4f Vec4f::operator != (const Vec4f& b) const
{
    return _mm_cmpneq_ps(v, b.v);
}

const Vec4f Vec4f::Dot2V(const Vec4f& v1, const Vec4f& v2)
{
    return _mm_dp_ps(v1, v2, 0x3F);
}

const Vec4f Vec4f::Dot3V(const Vec4f& v1, const Vec4f& v2)
{
    return _mm_dp_ps(v1, v2, 0x7F);
}

const Vec4f Vec4f::Dot4V(const Vec4f& v1, const Vec4f& v2)
{
    return _mm_dp_ps(v1, v2, 0xFF);
}

float Vec4f::Dot2(const Vec4f& v1, const Vec4f& v2)
{
    return _mm_cvtss_f32(Dot2V(v1, v2));
}

float Vec4f::Dot3(const Vec4f& v1, const Vec4f& v2)
{
    return _mm_cvtss_f32(Dot3V(v1, v2));
}

float Vec4f::Dot4(const Vec4f& v1, const Vec4f& v2)
{
    return _mm_cvtss_f32(Dot4V(v1, v2));
}

const Vec4f Vec4f::Cross3(const Vec4f& v1, const Vec4f& v2)
{
    __m128 vTemp1 = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 vTemp2 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
    vTemp1 = _mm_shuffle_ps(vTemp1, vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
    return NegMulAndAdd(vTemp1, vTemp2, vResult);
}

float Vec4f::Length2() const
{
    return _mm_cvtss_f32(Length2V());
}

const Vec4f Vec4f::Length2V() const
{
    return _mm_sqrt_ps(Dot2V(v, v));
}

float Vec4f::Length3() const
{
    return _mm_cvtss_f32(Length3V());
}

float Vec4f::SqrLength3() const
{
    return Dot3(*this, *this);
}

const Vec4f Vec4f::Length3V() const
{
    return _mm_sqrt_ps(Dot3V(v, v));
}

Vec4f& Vec4f::Normalize3()
{
    const __m128 vDot = Dot3V(v, v);
    const __m128 vTemp = _mm_sqrt_ps(vDot);
    v = _mm_div_ps(v, vTemp);
    return *this;
}

Vec4f& Vec4f::FastNormalize3()
{
    const __m128 vDot = Dot3V(v, v);
    v = _mm_mul_ps(v, _mm_rsqrt_ps(vDot));
    return *this;
}

float Vec4f::Length4() const
{
    return _mm_cvtss_f32(Length4V());
}

const Vec4f Vec4f::Length4V() const
{
    return _mm_sqrt_ps(Dot4V(v, v));
}

Vec4f& Vec4f::Normalize4()
{
    const __m128 vDot = Dot4V(v, v);
    const __m128 vTemp = _mm_sqrt_ps(vDot);
    v = _mm_div_ps(v, vTemp);
    return *this;
}

const Vec4f Vec4f::Fmod1(const Vec4f& v)
{
    return _mm_sub_ps(v, _mm_round_ps(v, _MM_FROUND_TO_ZERO));
}

// Check if any component is NaN
const VecBool4f Vec4f::IsNaN() const
{
    // check if exponent is all ones
    const __m128i epxMask = _mm_set1_epi32(0x7F800000);
    const __m128i expCheck = _mm_cmpeq_epi32(_mm_and_si128(vi, epxMask), epxMask);
    // check if mantissa is not zero
    const __m128i mantissaMask = _mm_set1_epi32(0x007FFFFF);
    const __m128i mantissaCheck = _mm_cmpeq_epi32(_mm_and_si128(vi, mantissaMask), _mm_setzero_si128());

    return _mm_andnot_si128(mantissaCheck, expCheck);
}

const VecBool4f Vec4f::IsInfinite() const
{
    // Mask off the sign bit
    __m128 temp = _mm_and_ps(v, VECTOR_MASK_ABS);
    // Compare to infinity
    return _mm_cmpeq_ps(temp, VECTOR_INF);
}

bool Vec4f::IsValid() const
{
    // check if exponent is all ones
    const __m128i epxMask = _mm_set1_epi32(0x7F800000);
    const __m128i expCheck = _mm_cmpeq_epi32(_mm_and_si128(vi, epxMask), epxMask);
    return _mm_test_all_zeros(expCheck, expCheck);
}

void Vec4f::Transpose3(Vec4f& a, Vec4f& b, Vec4f& c)
{
    const __m128 t0 = _mm_unpacklo_ps(a, b);
    const __m128 t1 = _mm_unpackhi_ps(a, b);
    a = _mm_movelh_ps(t0, c);
    b = _mm_shuffle_ps(t0, c, _MM_SHUFFLE(3, 1, 3, 2));
    c = _mm_shuffle_ps(t1, c, _MM_SHUFFLE(3, 2, 1, 0));
}

} // namespace Math
} // namespace NFE
