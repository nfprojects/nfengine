#pragma once

#include "Vec8f.hpp"

namespace NFE {
namespace Math {

VecBool8i::VecBool8i(const VecBool8f& other)
#ifdef NFE_USE_AVX512
    : mask(other.mask)
#else
    : v(_mm256_castps_si256(other.v))
#endif
{}

VecBool8i::VecBool8i(bool scalar)
{
#ifdef NFE_USE_AVX512
    mask = _cvtu32_mask8(scalar ? 0xFF : 0);
#else
    v = _mm256_set1_epi32(scalar ? -1 : 0);
#endif
}

VecBool8i::VecBool8i(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
{
#ifdef NFE_USE_AVX512
    mask = _cvtu32_mask8(uint16(e0) | e1 << 1 | e2 << 2 | e3 << 3 | e4 << 4 | e5 << 5 | e6 << 6 | e7 << 7);
#else
    v = _mm256_set_epi32(
        e7 ? -1 : 0,
        e6 ? -1 : 0,
        e5 ? -1 : 0,
        e4 ? -1 : 0,
        e3 ? -1 : 0,
        e2 ? -1 : 0,
        e1 ? -1 : 0,
        e0 ? -1 : 0
    );
#endif
}

template<uint32 index>
bool VecBool8i::Get() const
{
    static_assert(index < 8, "Invalid index");
#ifdef NFE_USE_AVX512
    return (uint32(mask) & (1 << index)) != 0;
#else
    return _mm256_extract_epi32(v, index) != 0;
#endif
}

uint32 VecBool8i::GetMask() const
{
#ifdef NFE_USE_AVX512
    return uint32(mask);
#else
    return _mm256_movemask_ps(_mm256_castsi256_ps(v));
#endif
}

bool VecBool8i::All() const
{
#ifdef NFE_USE_AVX512
    return mask == 0xFF;
#else
    return _mm256_testc_si256(v, _mm256_set1_epi32(-1));
#endif
}

bool VecBool8i::None() const
{
#ifdef NFE_USE_AVX512
    return _mm512_testz_or_mask8(mask, mask) != 0;
#else
    return _mm256_testz_si256(v, v) != 0;
#endif
}

bool VecBool8i::Any() const
{
#ifdef NFE_USE_AVX512
    return _mm512_testz_or_mask8(mask, mask) == 0;
#else
    return _mm256_testz_si256(v, v) == 0;
#endif
}

const VecBool8i VecBool8i::operator & (const VecBool8i rhs) const
{
#ifdef NFE_USE_AVX512
    return _kand_mask8(mask, rhs.mask);
#else
    return _mm256_and_si256(v, rhs.v);
#endif
}

const VecBool8i VecBool8i::operator | (const VecBool8i rhs) const
{
#ifdef NFE_USE_AVX512
    return _kor_mask8(mask, rhs.mask);
#else
    return _mm256_or_si256(v, rhs.v);
#endif
}

const VecBool8i VecBool8i::operator ^ (const VecBool8i rhs) const
{
#ifdef NFE_USE_AVX512
    return _kxor_mask8(mask, rhs.mask);
#else
    return _mm256_xor_si256(v, rhs.v);
#endif
}

bool VecBool8i::operator == (const VecBool8i& other) const
{
#ifdef NFE_USE_AVX512
    return mask == other.mask;
#else
    return _mm256_testc_si256(v, other.v) != 0;
#endif
}

bool VecBool8i::operator != (const VecBool8i& other) const
{
#ifdef NFE_USE_AVX512
    return mask != other.mask;
#else
    return _mm256_testc_si256(v, other.v) == 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec8i Vec8i::Zero()
{
    return _mm256_setzero_si256();
}

Vec8i::Vec8i(const Vec8i& other)
    : v(other.v)
{}

Vec8i::Vec8i(const Vec8ui& other)
    : v(other.v)
{}

Vec8i& Vec8i::operator = (const Vec8i& other)
{
    v = other.v;
    return *this;
}

Vec8i::Vec8i(const __m256i& m)
    : v(m)
{}

Vec8i::Vec8i(const __m256& m)
    : f(m)
{}

Vec8i::Vec8i(const Vec4i& lo, const Vec4i& hi)
    : v(_mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1u))
{}

const Vec8i Vec8i::Cast(const Vec8f& v)
{
    return _mm256_castps_si256(v);
}

const Vec8f Vec8i::AsVec8f() const
{
    return _mm256_castsi256_ps(v);
}

Vec8i::Vec8i(const int32 e0, const int32 e1, const int32 e2, const int32 e3, const int32 e4, const int32 e5, const int32 e6, const int32 e7)
    : v(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0))
{}

Vec8i::Vec8i(const int32 scalar)
    : v(_mm256_set1_epi32(scalar))
{}

Vec8i::Vec8i(const int32* scalarPtr)
    : v(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(scalarPtr)))
{}

const Vec8i Vec8i::Iota(const int32 value)
{
    return Vec8i(value) + Vec8i(0, 1, 2, 3, 4, 5, 6, 7);
}

const Vec8i Vec8i::Select(const Vec8i& a, const Vec8i& b, const VecBool8i& sel)
{
#if defined(NFE_USE_AVX512)
    return _mm256_mask_blend_epi32(sel, a, b);
#else
    return Vec8i(_mm256_blendv_ps(a.f, b.f, sel));
#endif
}

const Vec8i Vec8i::operator & (const Vec8i& b) const
{
    return Vec8i(_mm256_and_si256(v, b.v));
}

const Vec8i Vec8i::operator | (const Vec8i& b) const
{
    return Vec8i(_mm256_or_si256(v, b.v));
}

const Vec8i Vec8i::operator ^ (const Vec8i& b) const
{
    return Vec8i(_mm256_xor_si256(v, b.v));
}

Vec8i& Vec8i::operator &= (const Vec8i& b)
{
    v = _mm256_and_si256(v, b.v);
    return *this;
}

Vec8i& Vec8i::operator |= (const Vec8i& b)
{
    v = _mm256_or_si256(v, b.v);
    return *this;
}

Vec8i& Vec8i::operator ^= (const Vec8i& b)
{
    v = _mm256_xor_si256(v, b.v);
    return *this;
}

const Vec8i Vec8i::Convert(const Vec8f& v)
{
    return _mm256_cvtps_epi32(_mm256_round_ps(v, _MM_FROUND_TO_ZERO));
}

const Vec8f Vec8i::ConvertToVec8f() const
{
    return _mm256_cvtepi32_ps(v);
}

const Vec8i Vec8i::operator - () const
{
    return Vec8i::Zero() - (*this);
}

const Vec8i Vec8i::operator + (const Vec8i& b) const
{
    return _mm256_add_epi32(v, b);
}

const Vec8i Vec8i::operator - (const Vec8i& b) const
{
    return _mm256_sub_epi32(v, b);
}

const Vec8i Vec8i::operator * (const Vec8i& b) const
{
    return _mm256_mullo_epi32(v, b);
}

Vec8i& Vec8i::operator += (const Vec8i& b)
{
    v = _mm256_add_epi32(v, b);
    return *this;
}

Vec8i& Vec8i::operator -= (const Vec8i& b)
{
    v = _mm256_sub_epi32(v, b);
    return *this;
}

Vec8i& Vec8i::operator *= (const Vec8i& b)
{
    v = _mm256_mullo_epi32(v, b);
    return *this;
}

const Vec8i Vec8i::operator + (int32 b) const
{
    return _mm256_add_epi32(v, _mm256_set1_epi32(b));
}

const Vec8i Vec8i::operator - (int32 b) const
{
    return _mm256_sub_epi32(v, _mm256_set1_epi32(b));
}

const Vec8i Vec8i::operator * (int32 b) const
{
    return _mm256_mullo_epi32(v, _mm256_set1_epi32(b));
}

Vec8i& Vec8i::operator += (int32 b)
{
    v = _mm256_add_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

Vec8i& Vec8i::operator -= (int32 b)
{
    v = _mm256_sub_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

Vec8i& Vec8i::operator *= (int32 b)
{
    v = _mm256_mullo_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

const VecBool8i Vec8i::operator == (const Vec8i& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_epi32_mask(v, b.v, _MM_CMPINT_EQ);
#else
    return _mm256_cmpeq_epi32(v, b.v);
#endif
}

const VecBool8i Vec8i::operator != (const Vec8i& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_epi32_mask(v, b.v, _MM_CMPINT_NE);
#else
    // TODO may not be optimal, when used with selector later on
    // idea: have VecNegBool or something
    return _mm256_xor_si256(_mm256_set1_epi32(-1), _mm256_cmpeq_epi32(v, b.v));
#endif
}

const Vec8i Vec8i::operator << (const Vec8i& b) const
{
    return _mm256_sllv_epi32(v, b);
}

const Vec8i Vec8i::operator >> (const Vec8i& b) const
{
    return _mm256_srav_epi32(v, b);
}

const Vec8i Vec8i::operator << (int32 b) const
{
    return _mm256_slli_epi32(v, b);
}

const Vec8i Vec8i::operator >> (int32 b) const
{
    return _mm256_srai_epi32(v, b);
}

const Vec8i Vec8i::Min(const Vec8i& a, const Vec8i& b)
{
    return _mm256_min_epi32(a, b);
}

const Vec8i Vec8i::Max(const Vec8i& a, const Vec8i& b)
{
    return _mm256_max_epi32(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec8ui Vec8ui::Zero()
{
    return _mm256_setzero_si256();
}

Vec8ui::Vec8ui(const Vec8ui& other)
    : v(other.v)
{}

Vec8ui::Vec8ui(const Vec8i& other)
    : v(other.v)
{}

Vec8ui& Vec8ui::operator = (const Vec8ui& other)
{
    v = other.v;
    return *this;
}

Vec8ui::Vec8ui(const __m256i& m)
    : v(m)
{}

Vec8ui::Vec8ui(const __m256& m)
    : f(m)
{}

Vec8ui::Vec8ui(const Vec4ui& lo, const Vec4ui& hi)
    : v(_mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1u))
{}

const Vec8ui Vec8ui::Cast(const Vec8f& v)
{
    return _mm256_castps_si256(v);
}

const Vec8f Vec8ui::AsVec8f() const
{
    return _mm256_castsi256_ps(v);
}

Vec8ui::Vec8ui(const uint32 e0, const uint32 e1, const uint32 e2, const uint32 e3, const uint32 e4, const uint32 e5, const uint32 e6, const uint32 e7)
    : v(_mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0))
{}

Vec8ui::Vec8ui(const uint32 scalar)
    : v(_mm256_set1_epi32(static_cast<int32>(scalar)))
{}

Vec8ui::Vec8ui(const uint32* scalarPtr)
    : v(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(scalarPtr)))
{}

const Vec8ui Vec8ui::Iota(const uint32 value)
{
    return Vec8ui(value) + Vec8ui(0, 1, 2, 3, 4, 5, 6, 7);
}

const Vec8ui Vec8ui::Select(const Vec8ui& a, const Vec8ui& b, const VecBool8i& sel)
{
#if defined(NFE_USE_AVX512)
    return _mm256_mask_blend_epi32(sel, a, b);
#else
    return Vec8ui(_mm256_blendv_ps(a.f, b.f, sel));
#endif
}

const Vec8ui Vec8ui::operator & (const Vec8ui& b) const
{
    return Vec8ui(_mm256_and_si256(v, b.v));
}

const Vec8ui Vec8ui::operator | (const Vec8ui& b) const
{
    return Vec8ui(_mm256_or_si256(v, b.v));
}

const Vec8ui Vec8ui::operator ^ (const Vec8ui& b) const
{
    return Vec8ui(_mm256_xor_si256(v, b.v));
}

Vec8ui& Vec8ui::operator &= (const Vec8ui& b)
{
    v = _mm256_and_si256(v, b.v);
    return *this;
}

Vec8ui& Vec8ui::operator |= (const Vec8ui& b)
{
    v = _mm256_or_si256(v, b.v);
    return *this;
}

Vec8ui& Vec8ui::operator ^= (const Vec8ui& b)
{
    v = _mm256_xor_si256(v, b.v);
    return *this;
}

const Vec8ui Vec8ui::operator + (const Vec8ui& b) const
{
    return _mm256_add_epi32(v, b);
}

const Vec8ui Vec8ui::operator - (const Vec8ui& b) const
{
    return _mm256_sub_epi32(v, b);
}

Vec8ui& Vec8ui::operator += (const Vec8ui& b)
{
    v = _mm256_add_epi32(v, b);
    return *this;
}

Vec8ui& Vec8ui::operator -= (const Vec8ui& b)
{
    v = _mm256_sub_epi32(v, b);
    return *this;
}

const Vec8ui Vec8ui::operator + (int32 b) const
{
    return _mm256_add_epi32(v, _mm256_set1_epi32(b));
}

const Vec8ui Vec8ui::operator - (int32 b) const
{
    return _mm256_sub_epi32(v, _mm256_set1_epi32(b));
}

Vec8ui& Vec8ui::operator += (int32 b)
{
    v = _mm256_add_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

Vec8ui& Vec8ui::operator -= (int32 b)
{
    v = _mm256_sub_epi32(v, _mm256_set1_epi32(b));
    return *this;
}

const VecBool8i Vec8ui::operator == (const Vec8ui& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_epi32_mask(v, b.v, _MM_CMPINT_EQ);
#else
    return _mm256_cmpeq_epi32(v, b.v);
#endif
}

const VecBool8i Vec8ui::operator != (const Vec8ui& b) const
{
#ifdef NFE_USE_AVX512
    return _mm256_cmp_epi32_mask(v, b.v, _MM_CMPINT_NE);
#else
    // TODO may not be optimal, when used with selector later on
    // idea: have VecNegBool or something
    return _mm256_xor_si256(_mm256_set1_epi32(-1), _mm256_cmpeq_epi32(v, b.v));
#endif
}

const Vec8ui Vec8ui::operator << (const Vec8ui& b) const
{
    return _mm256_sllv_epi32(v, b);
}

const Vec8ui Vec8ui::operator >> (const Vec8ui& b) const
{
    return _mm256_srlv_epi32(v, b);
}

const Vec8ui Vec8ui::operator << (int32 b) const
{
    return _mm256_slli_epi32(v, b);
}

const Vec8ui Vec8ui::operator >> (int32 b) const
{
    return _mm256_srli_epi32(v, b);
}

const Vec8ui Vec8ui::Min(const Vec8ui& a, const Vec8ui& b)
{
    return _mm256_min_epu32(a, b);
}

const Vec8ui Vec8ui::Max(const Vec8ui& a, const Vec8ui& b)
{
    return _mm256_max_epu32(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec8f Gather8(const float* basePtr, const Vec8i& indices)
{
    return _mm256_i32gather_ps(basePtr, indices, 4);
}

} // namespace Math
} // namespace NFE
