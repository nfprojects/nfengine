#pragma once

#include "Vec4f.hpp"

namespace NFE {
namespace Math {

VecBool4i::VecBool4i(bool x, bool y, bool z, bool w)
{
    v = _mm_set_epi32(w ? 0xFFFFFFFF : 0, z ? 0xFFFFFFFF : 0, y ? 0xFFFFFFFF : 0, x ? 0xFFFFFFFF : 0);
}

VecBool4i::VecBool4i(const VecBool4f& other)
{
    v = _mm_castps_si128(other.v);
}

VecBool4i::operator VecBool4f()
{
    return v;
}

template<uint32 index>
bool VecBool4i::Get() const
{
    static_assert(index < 4, "Invalid index");
    return _mm_extract_ps(v, index) != 0;
}

bool VecBool4i::All() const
{
    return _mm_test_all_ones(v) != 0;
}

bool VecBool4i::None() const
{
    return _mm_testz_si128(v, v) != 0;
}

bool VecBool4i::Any() const
{
    return _mm_testz_si128(v, v) == 0;
}

bool VecBool4i::All3() const
{
    return (_mm_movemask_epi8(v) & 0xFFF) == 0xFFF;
}

bool VecBool4i::None3() const
{
    return (_mm_movemask_epi8(v) & 0xFFF) == 0;
}

bool VecBool4i::Any3() const
{
    return (_mm_movemask_epi8(v) & 0xFFF) != 0;
}

const VecBool4i VecBool4i::operator & (const VecBool4i rhs) const
{
    return _mm_and_si128(v, rhs.v);
}

const VecBool4i VecBool4i::operator | (const VecBool4i rhs) const
{
    return _mm_or_si128(v, rhs.v);
}

const VecBool4i VecBool4i::operator ^ (const VecBool4i rhs) const
{
    return _mm_xor_si128(v, rhs.v);
}

bool VecBool4i::operator == (const VecBool4i& other) const
{
    return _mm_testc_si128(v, other.v) != 0;
}

bool VecBool4i::operator != (const VecBool4i& other) const
{
    return _mm_testc_si128(v, other.v) == 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec4i Vec4i::Zero()
{
    return _mm_setzero_si128();
}

const Vec4i Vec4i::Iota(const int32 value)
{
    return Vec4i(value) + Vec4i(0, 1, 2, 3);
}

Vec4i::Vec4i(const __m128i& m)
    : v(m)
{}

Vec4i::Vec4i(const Vec4i& other)
    : v(other.v)
{}

Vec4i::Vec4i(const Vec4ui& other)
    : v(other.v)
{}

Vec4i::Vec4i(const VecBool4i& other)
    : v(other.v)
{}

Vec4i& Vec4i::operator = (const Vec4i& other)
{
    v = other.v;
    return *this;
}

const Vec4i Vec4i::Cast(const Vec4f& v)
{
    return _mm_castps_si128(v);
}

const Vec4f Vec4i::AsVec4f() const
{
    return _mm_castsi128_ps(v);
}

Vec4i::Vec4i(const int32 x, const int32 y, const int32 z, const int32 w)
    : v(_mm_set_epi32(w, z, y, x))
{}

Vec4i::Vec4i(const int32 i)
    : v(_mm_set1_epi32(i))
{}

Vec4i::Vec4i(const int32* i)
    : v(_mm_load_epi32(i))
{}

const Vec4i Vec4i::Convert(const Vec4f& v)
{
    return _mm_cvtps_epi32(v);
}

const Vec4i Vec4i::TruncateAndConvert(const Vec4f& v)
{
    return _mm_cvttps_epi32(v);
}

const Vec4f Vec4i::ConvertToVec4f() const
{
    return _mm_cvtepi32_ps(v);
}

const Vec4i Vec4i::Select(const Vec4i& a, const Vec4i& b, const VecBool4i& sel)
{
    return _mm_blendv_epi8(a, b, sel.v);
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vec4i Vec4i::Select(const Vec4i& a, const Vec4i& b)
{
    static_assert(selX <= 1, "Invalid X index");
    static_assert(selY <= 1, "Invalid Y index");
    static_assert(selZ <= 1, "Invalid Z index");
    static_assert(selW <= 1, "Invalid W index");

    return _mm_blend_epi32(a, b, selX | (selY << 1) | (selZ << 2) | (selW << 3));
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4i Vec4i::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    if constexpr (ix == 0 && iy == 1 && iz == 2 && iw == 3)
    {
        return *this;
    }
    else if constexpr (ix == 0 && iy == 0 && iz == 1 && iw == 1)
    {
        return _mm_unpacklo_epi32(v, v);
    }
    else if constexpr (ix == 2 && iy == 2 && iz == 3 && iw == 3)
    {
        return _mm_unpackhi_epi32(v, v);
    }
    else if constexpr (ix == 0 && iy == 1 && iz == 0 && iw == 1)
    {
        return _mm_unpacklo_epi64(v, v);
    }
    else if constexpr (ix == 2 && iy == 3 && iz == 2 && iw == 3)
    {
        return _mm_unpackhi_epi64(v, v);
    }
    else
    {
        return _mm_shuffle_epi32(v, _MM_SHUFFLE(iw, iz, iy, ix));
    }
}

//////////////////////////////////////////////////////////////////////////

const Vec4i Vec4i::operator & (const Vec4i& b) const
{
    return _mm_and_si128(v, b.v);
}

const Vec4i Vec4i::AndNot(const Vec4i& a, const Vec4i& b)
{
    return _mm_andnot_si128(a.v, b.v);
}

const Vec4i Vec4i::operator | (const Vec4i& b) const
{
    return _mm_or_si128(v, b.v);
}

const Vec4i Vec4i::operator ^ (const Vec4i& b) const
{
    return _mm_xor_si128(v, b.v);
}

Vec4i& Vec4i::operator &= (const Vec4i& b)
{
    v = _mm_and_si128(v, b.v);
    return *this;
}

Vec4i& Vec4i::operator |= (const Vec4i& b)
{
    v = _mm_or_si128(v, b.v);
    return *this;
}

Vec4i& Vec4i::operator ^= (const Vec4i& b)
{
    v = _mm_xor_si128(v, b.v);
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec4i Vec4i::operator - () const
{
    return Vec4i::Zero() - (*this);
}

const Vec4i Vec4i::operator + (const Vec4i& b) const
{
    return _mm_add_epi32(v, b);
}

const Vec4i Vec4i::operator - (const Vec4i& b) const
{
    return _mm_sub_epi32(v, b);
}

const Vec4i Vec4i::operator * (const Vec4i& b) const
{
    return _mm_mullo_epi32(v, b);
}

Vec4i& Vec4i::operator += (const Vec4i& b)
{
    v = _mm_add_epi32(v, b);
    return *this;
}

Vec4i& Vec4i::operator -= (const Vec4i& b)
{
    v = _mm_sub_epi32(v, b);
    return *this;
}

Vec4i& Vec4i::operator *= (const Vec4i& b)
{
    v = _mm_mullo_epi32(v, b);
    return *this;
}

const Vec4i Vec4i::operator + (int32 b) const
{
    return _mm_add_epi32(v, _mm_set1_epi32(b));
}

const Vec4i Vec4i::operator - (int32 b) const
{
    return _mm_sub_epi32(v, _mm_set1_epi32(b));
}

const Vec4i Vec4i::operator * (int32 b) const
{
    return _mm_mullo_epi32(v, _mm_set1_epi32(b));
}

Vec4i& Vec4i::operator += (int32 b)
{
    v = _mm_add_epi32(v, _mm_set1_epi32(b));
    return *this;
}

Vec4i& Vec4i::operator -= (int32 b)
{
    v = _mm_sub_epi32(v, _mm_set1_epi32(b));
    return *this;
}

Vec4i& Vec4i::operator *= (int32 b)
{
    v = _mm_mullo_epi32(v, _mm_set1_epi32(b));
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec4i Vec4i::operator << (const Vec4i& b) const
{
#ifdef NFE_USE_AVX2
    return _mm_sllv_epi32(v, b);
#else
    return { x << b.x, y << b.y, z << b.z, w << b.w };
#endif
}

const Vec4i Vec4i::operator >> (const Vec4i& b) const
{
#ifdef NFE_USE_AVX2
    return _mm_srav_epi32(v, b);
#else
    return { x >> b.x, y >> b.y, z >> b.z, w >> b.w };
#endif
}

Vec4i& Vec4i::operator <<= (const Vec4i& b)
{
#ifdef NFE_USE_AVX2
    v = _mm_sllv_epi32(v, b);
#else
    x <<= b.x;
    y <<= b.y;
    z <<= b.z;
    w <<= b.w;
#endif
    return *this;
}

Vec4i& Vec4i::operator >>= (const Vec4i& b)
{
#ifdef NFE_USE_AVX2
    v = _mm_srav_epi32(v, b);
#else
    x >>= b.x;
    y >>= b.y;
    z >>= b.z;
    w >>= b.w;
#endif
    return *this;
}

const Vec4i Vec4i::operator << (int32 b) const
{
    return _mm_slli_epi32(v, b);
}

const Vec4i Vec4i::operator >> (int32 b) const
{
    return _mm_srai_epi32(v, b);
}

Vec4i& Vec4i::operator <<= (int32 b)
{
    v = _mm_slli_epi32(v, b);
    return *this;
}

Vec4i& Vec4i::operator >>= (int32 b)
{
    v = _mm_srai_epi32(v, b);
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VecBool4i Vec4i::operator == (const Vec4i& b) const
{
    return _mm_cmpeq_epi32(v, b.v);
}

const VecBool4i Vec4i::operator != (const Vec4i& b) const
{
    return _mm_xor_si128(_mm_set1_epi32(0xFFFFFFFF), _mm_cmpeq_epi32(v, b.v));
}

const VecBool4i Vec4i::operator < (const Vec4i& b) const
{
    return _mm_cmplt_epi32(v, b.v);
}

const VecBool4i Vec4i::operator > (const Vec4i& b) const
{
    return _mm_cmpgt_epi32(v, b.v);
}

const VecBool4i Vec4i::operator >= (const Vec4i& b) const
{
    return _mm_xor_si128(_mm_set1_epi32(0xFFFFFFFF), _mm_cmplt_epi32(v, b.v));
}

const VecBool4i Vec4i::operator <= (const Vec4i& b) const
{
    return _mm_xor_si128(_mm_set1_epi32(0xFFFFFFFF), _mm_cmpgt_epi32(v, b.v));
}

//////////////////////////////////////////////////////////////////////////

const Vec4i Vec4i::Min(const Vec4i& a, const Vec4i& b)
{
    return _mm_min_epi32(a, b);
}

const Vec4i Vec4i::Max(const Vec4i& a, const Vec4i& b)
{
    return _mm_max_epi32(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::Zero()
{
    return _mm_setzero_si128();
}

Vec4ui::Vec4ui(const __m128i& m)
    : v(m)
{}

Vec4ui::Vec4ui(const Vec4ui& other)
    : v(other.v)
{}

Vec4ui::Vec4ui(const Vec4i& other)
    : v(other.v)
{}

Vec4ui& Vec4ui::operator = (const Vec4ui& other)
{
    v = other.v;
    return *this;
}

const Vec4ui Vec4ui::Cast(const Vec4f& v)
{
    return _mm_castps_si128(v);
}

const Vec4f Vec4ui::AsVec4f() const
{
    return _mm_castsi128_ps(v);
}

const Vec4ui Vec4ui::Convert(const Vec4f& v)
{
    // TODO this is not correct
    return _mm_cvtps_epi32(v);
}

const Vec4ui Vec4ui::TruncateAndConvert(const Vec4f& v)
{
    // TODO this is not correct
    return _mm_cvttps_epi32(v);
}

const Vec4f Vec4ui::ConvertToVec4f() const
{
    // TODO this is not correct
    return _mm_cvtepi32_ps(v);
}

Vec4ui::Vec4ui(const uint32 x, const uint32 y, const uint32 z, const uint32 w)
    : v(_mm_set_epi32(w, z, y, x))
{}

Vec4ui::Vec4ui(const uint32 i)
    : v(_mm_set1_epi32(i))
{}

Vec4ui::Vec4ui(const uint32* i)
    : v(_mm_load_epi32(i))
{}

const Vec4ui Vec4ui::Iota(const uint32 value)
{
    return Vec4ui(value) + Vec4ui(0, 1, 2, 3);
}

const Vec4ui Vec4ui::Select(const Vec4ui& a, const Vec4ui& b, const VecBool4i& sel)
{
    return _mm_blendv_epi8(a, b, sel.v);
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec4ui Vec4ui::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    if constexpr (ix == 0 && iy == 1 && iz == 2 && iw == 3)
    {
        return *this;
    }
    else if constexpr (ix == 0 && iy == 0 && iz == 1 && iw == 1)
    {
        return _mm_unpacklo_epi32(v, v);
    }
    else if constexpr (ix == 2 && iy == 2 && iz == 3 && iw == 3)
    {
        return _mm_unpackhi_epi32(v, v);
    }
    else if constexpr (ix == 0 && iy == 1 && iz == 0 && iw == 1)
    {
        return _mm_unpacklo_epi64(v, v);
    }
    else if constexpr (ix == 2 && iy == 3 && iz == 2 && iw == 3)
    {
        return _mm_unpackhi_epi64(v, v);
    }
    else
    {
        return _mm_shuffle_epi32(v, _MM_SHUFFLE(iw, iz, iy, ix));
    }
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::operator & (const Vec4ui& b) const
{
    return _mm_and_si128(v, b.v);
}

const Vec4ui Vec4ui::AndNot(const Vec4ui& a, const Vec4ui& b)
{
    return _mm_andnot_si128(a.v, b.v);
}

const Vec4ui Vec4ui::operator | (const Vec4ui& b) const
{
    return _mm_or_si128(v, b.v);
}

const Vec4ui Vec4ui::operator ^ (const Vec4ui& b) const
{
    return _mm_xor_si128(v, b.v);
}

Vec4ui& Vec4ui::operator &= (const Vec4ui& b)
{
    v = _mm_and_si128(v, b.v);
    return *this;
}

Vec4ui& Vec4ui::operator |= (const Vec4ui& b)
{
    v = _mm_or_si128(v, b.v);
    return *this;
}

Vec4ui& Vec4ui::operator ^= (const Vec4ui& b)
{
    v = _mm_xor_si128(v, b.v);
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::operator - () const
{
    return Vec4ui::Zero() - (*this);
}

const Vec4ui Vec4ui::operator + (const Vec4ui& b) const
{
    return _mm_add_epi32(v, b);
}

const Vec4ui Vec4ui::operator - (const Vec4ui& b) const
{
    return _mm_sub_epi32(v, b);
}

const Vec4ui Vec4ui::operator * (const Vec4ui& b) const
{
    return _mm_mullo_epi32(v, b);
}

Vec4ui& Vec4ui::operator += (const Vec4ui& b)
{
    v = _mm_add_epi32(v, b);
    return *this;
}

Vec4ui& Vec4ui::operator -= (const Vec4ui& b)
{
    v = _mm_sub_epi32(v, b);
    return *this;
}

Vec4ui& Vec4ui::operator *= (const Vec4ui& b)
{
    v = _mm_mullo_epi32(v, b);
    return *this;
}

const Vec4ui Vec4ui::operator + (uint32 b) const
{
    return _mm_add_epi32(v, _mm_set1_epi32(b));
}

const Vec4ui Vec4ui::operator - (uint32 b) const
{
    return _mm_sub_epi32(v, _mm_set1_epi32(b));
}

const Vec4ui Vec4ui::operator * (uint32 b) const
{
    return _mm_mullo_epi32(v, _mm_set1_epi32(b));
}

Vec4ui& Vec4ui::operator += (uint32 b)
{
    v = _mm_add_epi32(v, _mm_set1_epi32(b));
    return *this;
}

Vec4ui& Vec4ui::operator -= (uint32 b)
{
    v = _mm_sub_epi32(v, _mm_set1_epi32(b));
    return *this;
}

Vec4ui& Vec4ui::operator *= (uint32 b)
{
    v = _mm_mullo_epi32(v, _mm_set1_epi32(b));
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::operator << (const Vec4ui& b) const
{
#ifdef NFE_USE_AVX2
    return _mm_sllv_epi32(v, b);
#else
    return { x << b.x, y << b.y, z << b.z, w << b.w };
#endif
}

const Vec4ui Vec4ui::operator >> (const Vec4ui& b) const
{
#ifdef NFE_USE_AVX2
    return _mm_srlv_epi32(v, b);
#else
    return { x >> b.x, y >> b.y, z >> b.z, w >> b.w };
#endif
}

Vec4ui& Vec4ui::operator <<= (const Vec4ui& b)
{
#ifdef NFE_USE_AVX2
    v = _mm_sllv_epi32(v, b);
#else
    x <<= b.x;
    y <<= b.y;
    z <<= b.z;
    w <<= b.w;
#endif
    return *this;
}

Vec4ui& Vec4ui::operator >>= (const Vec4ui& b)
{
#ifdef NFE_USE_AVX2
    v = _mm_srlv_epi32(v, b);
#else
    x >>= b.x;
    y >>= b.y;
    z >>= b.z;
    w >>= b.w;
#endif
    return *this;
}

const Vec4ui Vec4ui::operator << (uint32 b) const
{
    return _mm_slli_epi32(v, b);
}

const Vec4ui Vec4ui::operator >> (uint32 b) const
{
    return _mm_srli_epi32(v, b);
}

Vec4ui& Vec4ui::operator <<= (uint32 b)
{
    v = _mm_slli_epi32(v, b);
    return *this;
}

Vec4ui& Vec4ui::operator >>= (uint32 b)
{
    v = _mm_srli_epi32(v, b);
    return *this;
}

//////////////////////////////////////////////////////////////////////////

const VecBool4i Vec4ui::operator == (const Vec4ui& b) const
{
    return _mm_cmpeq_epi32(v, b.v);
}

const VecBool4i Vec4ui::operator != (const Vec4ui& b) const
{
    return _mm_xor_si128(_mm_set1_epi32(0xFFFFFFFF), _mm_cmpeq_epi32(v, b.v));
}

const VecBool4i Vec4ui::operator < (const Vec4ui& b) const
{
    const __m128i signbit = _mm_set1_epi32(0x80000000);
    const __m128i a1 = _mm_xor_si128(v, signbit);
    const __m128i b1 = _mm_xor_si128(b.v, signbit);
    return _mm_cmpgt_epi32(b1, a1);
}

const VecBool4i Vec4ui::operator > (const Vec4ui& b) const
{
    const __m128i signbit = _mm_set1_epi32(0x80000000);
    const __m128i a1 = _mm_xor_si128(v, signbit);
    const __m128i b1 = _mm_xor_si128(b.v, signbit);
    return _mm_cmpgt_epi32(a1, b1);
}

const VecBool4i Vec4ui::operator >= (const Vec4ui& b) const
{
    __m128i max_ab = _mm_max_epu32(v, b.v);
    return _mm_cmpeq_epi32(v, max_ab);
}

const VecBool4i Vec4ui::operator <= (const Vec4ui& b) const
{
    __m128i max_ab = _mm_max_epu32(v, b.v);
    return _mm_cmpeq_epi32(max_ab, v);
}

//////////////////////////////////////////////////////////////////////////

const Vec4ui Vec4ui::Min(const Vec4ui& a, const Vec4ui& b)
{
    return _mm_min_epu32(a, b);
}

const Vec4ui Vec4ui::Max(const Vec4ui& a, const Vec4ui& b)
{
    return _mm_max_epu32(a, b);
}

} // namespace Math
} // namespace NFE
