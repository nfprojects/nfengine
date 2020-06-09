#pragma once

#include "Vec16i.hpp"
#include "Vec16f.hpp"

namespace NFE {
namespace Math {

const Vec16i Vec16i::Zero()
{
    return _mm512_setzero_si512();
}

Vec16i::Vec16i(const Vec16i& other)
    : v(other.v)
{}

Vec16i::Vec16i(const Vec16ui& other)
    : v(other.v)
{}

Vec16i& Vec16i::operator = (const Vec16i& other)
{
    v = other.v;
    return *this;
}

Vec16i::Vec16i(const __m512i& m)
    : v(m)
{}

const Vec16i Vec16i::Cast(const Vec16f& v)
{
    return _mm512_castps_si512(v);
}

const Vec16f Vec16i::AsVec16f() const
{
    return _mm512_castsi512_ps(v);
}

Vec16i::Vec16i(
    const int32 e0, const int32 e1, const int32 e2, const int32 e3,
    const int32 e4, const int32 e5, const int32 e6, const int32 e7,
    const int32 e8, const int32 e9, const int32 e10, const int32 e11,
    const int32 e12, const int32 e13, const int32 e14, const int32 e15
)
    : v(_mm512_setr_epi32(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15))
{}

Vec16i::Vec16i(const int32 scalar)
    : v(_mm512_set1_epi32(scalar))
{}

Vec16i::Vec16i(const int32* scalarPtr)
    : v(_mm512_loadu_si512(reinterpret_cast<const __m512i*>(scalarPtr)))
{}

const Vec16i Vec16i::Iota(const int32 value)
{
    return Vec16i(value) + Vec16i(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
}

const Vec16i Vec16i::Select(const Vec16i& a, const Vec16i& b, const VecBool16& sel)
{
    return Vec16i(_mm512_mask_blend_epi32(sel, a, b));
}

const Vec16i Vec16i::operator & (const Vec16i& b) const
{
    return Vec16i(_mm512_and_si512(v, b.v));
}

const Vec16i Vec16i::operator | (const Vec16i& b) const
{
    return Vec16i(_mm512_or_si512(v, b.v));
}

const Vec16i Vec16i::operator ^ (const Vec16i& b) const
{
    return Vec16i(_mm512_xor_si512(v, b.v));
}

Vec16i& Vec16i::operator &= (const Vec16i& b)
{
    v = _mm512_and_si512(v, b.v);
    return *this;
}

Vec16i& Vec16i::operator |= (const Vec16i& b)
{
    v = _mm512_or_si512(v, b.v);
    return *this;
}

Vec16i& Vec16i::operator ^= (const Vec16i& b)
{
    v = _mm512_xor_si512(v, b.v);
    return *this;
}

const Vec16i Vec16i::Convert(const Vec16f& v)
{
    return _mm512_cvtps_epi32(v);
}

const Vec16f Vec16i::ConvertToVec16f() const
{
    return _mm512_cvtepi32_ps(v);
}

const Vec16i Vec16i::operator - () const
{
    return Vec16i::Zero() - (*this);
}

const Vec16i Vec16i::operator + (const Vec16i& b) const
{
    return _mm512_add_epi32(v, b);
}

const Vec16i Vec16i::operator - (const Vec16i& b) const
{
    return _mm512_sub_epi32(v, b);
}

const Vec16i Vec16i::operator * (const Vec16i& b) const
{
    return _mm512_mullo_epi32(v, b);
}

Vec16i& Vec16i::operator += (const Vec16i& b)
{
    v = _mm512_add_epi32(v, b);
    return *this;
}

Vec16i& Vec16i::operator -= (const Vec16i& b)
{
    v = _mm512_sub_epi32(v, b);
    return *this;
}

Vec16i& Vec16i::operator *= (const Vec16i& b)
{
    v = _mm512_mullo_epi32(v, b);
    return *this;
}

const Vec16i Vec16i::operator + (int32 b) const
{
    return _mm512_add_epi32(v, _mm512_set1_epi32(b));
}

const Vec16i Vec16i::operator - (int32 b) const
{
    return _mm512_sub_epi32(v, _mm512_set1_epi32(b));
}

const Vec16i Vec16i::operator * (int32 b) const
{
    return _mm512_mullo_epi32(v, _mm512_set1_epi32(b));
}

Vec16i& Vec16i::operator += (int32 b)
{
    v = _mm512_add_epi32(v, _mm512_set1_epi32(b));
    return *this;
}

Vec16i& Vec16i::operator -= (int32 b)
{
    v = _mm512_sub_epi32(v, _mm512_set1_epi32(b));
    return *this;
}

Vec16i& Vec16i::operator *= (int32 b)
{
    v = _mm512_mullo_epi32(v, _mm512_set1_epi32(b));
    return *this;
}

const VecBool16 Vec16i::operator == (const Vec16i& b) const
{
    return _mm512_cmpeq_epi32_mask(v, b.v);
}

const VecBool16 Vec16i::operator != (const Vec16i& b) const
{
    return _mm512_cmpneq_epi32_mask(v, b.v);
}

const Vec16i Vec16i::operator << (const Vec16i& b) const
{
    return _mm512_sllv_epi32(v, b);
}

const Vec16i Vec16i::operator >> (const Vec16i& b) const
{
    return _mm512_srav_epi32(v, b);
}

const Vec16i Vec16i::operator << (int32 b) const
{
    return _mm512_slli_epi32(v, b);
}

const Vec16i Vec16i::operator >> (int32 b) const
{
    return _mm512_srai_epi32(v, b);
}

const Vec16i Vec16i::Min(const Vec16i& a, const Vec16i& b)
{
    return _mm512_min_epi32(a, b);
}

const Vec16i Vec16i::Max(const Vec16i& a, const Vec16i& b)
{
    return _mm512_max_epi32(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec16ui Vec16ui::Zero()
{
    return _mm512_setzero_si512();
}

Vec16ui::Vec16ui(const Vec16ui& other)
    : v(other.v)
{}

Vec16ui::Vec16ui(const Vec16i& other)
    : v(other.v)
{}

Vec16ui& Vec16ui::operator = (const Vec16ui& other)
{
    v = other.v;
    return *this;
}

Vec16ui::Vec16ui(const __m512i& m)
    : v(m)
{}

const Vec16ui Vec16ui::Cast(const Vec16f& v)
{
    return _mm512_castps_si512(v);
}

const Vec16f Vec16ui::AsVec16f() const
{
    return _mm512_castsi512_ps(v);
}

Vec16ui::Vec16ui(
    const uint32 e0, const uint32 e1, const uint32 e2, const uint32 e3,
    const uint32 e4, const uint32 e5, const uint32 e6, const uint32 e7,
    const uint32 e8, const uint32 e9, const uint32 e10, const uint32 e11,
    const uint32 e12, const uint32 e13, const uint32 e14, const uint32 e15
)
    : v(_mm512_setr_epi32(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15))
{}

Vec16ui::Vec16ui(const uint32 scalar)
    : v(_mm512_set1_epi32(static_cast<int32>(scalar)))
{}

Vec16ui::Vec16ui(const uint32* scalarPtr)
    : v(_mm512_loadu_epi32(scalarPtr))
{}

const Vec16ui Vec16ui::Iota(const uint32 value)
{
    return Vec16ui(value) + Vec16ui(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
}

const Vec16ui Vec16ui::Select(const Vec16ui& a, const Vec16ui& b, const VecBool16& sel)
{
    return Vec16ui(_mm512_mask_blend_epi32(sel, a, b));
}

const Vec16ui Vec16ui::operator & (const Vec16ui& b) const
{
    return Vec16ui(_mm512_and_si512(v, b.v));
}

const Vec16ui Vec16ui::operator | (const Vec16ui& b) const
{
    return Vec16ui(_mm512_or_si512(v, b.v));
}

const Vec16ui Vec16ui::operator ^ (const Vec16ui& b) const
{
    return Vec16ui(_mm512_xor_si512(v, b.v));
}

Vec16ui& Vec16ui::operator &= (const Vec16ui& b)
{
    v = _mm512_and_si512(v, b.v);
    return *this;
}

Vec16ui& Vec16ui::operator |= (const Vec16ui& b)
{
    v = _mm512_or_si512(v, b.v);
    return *this;
}

Vec16ui& Vec16ui::operator ^= (const Vec16ui& b)
{
    v = _mm512_xor_si512(v, b.v);
    return *this;
}

const Vec16ui Vec16ui::operator + (const Vec16ui& b) const
{
    return _mm512_add_epi32(v, b);
}

const Vec16ui Vec16ui::operator - (const Vec16ui& b) const
{
    return _mm512_sub_epi32(v, b);
}

Vec16ui& Vec16ui::operator += (const Vec16ui& b)
{
    v = _mm512_add_epi32(v, b);
    return *this;
}

Vec16ui& Vec16ui::operator -= (const Vec16ui& b)
{
    v = _mm512_sub_epi32(v, b);
    return *this;
}

const Vec16ui Vec16ui::operator + (uint32 b) const
{
    return _mm512_add_epi32(v, _mm512_set1_epi32(b));
}

const Vec16ui Vec16ui::operator - (uint32 b) const
{
    return _mm512_sub_epi32(v, _mm512_set1_epi32(b));
}

Vec16ui& Vec16ui::operator += (uint32 b)
{
    v = _mm512_add_epi32(v, _mm512_set1_epi32(b));
    return *this;
}

Vec16ui& Vec16ui::operator -= (uint32 b)
{
    v = _mm512_sub_epi32(v, _mm512_set1_epi32(b));
    return *this;
}

const VecBool16 Vec16ui::operator == (const Vec16ui& b) const
{
    return _mm512_cmpeq_epu32_mask(v, b.v);
}

const VecBool16 Vec16ui::operator != (const Vec16ui& b) const
{
    return _mm512_cmpneq_epu32_mask(v, b.v);
}

const Vec16ui Vec16ui::operator << (const Vec16ui& b) const
{
    return _mm512_sllv_epi32(v, b);
}

const Vec16ui Vec16ui::operator >> (const Vec16ui& b) const
{
    return _mm512_srlv_epi32(v, b);
}

const Vec16ui Vec16ui::operator << (uint32 b) const
{
    return _mm512_slli_epi32(v, b);
}

const Vec16ui Vec16ui::operator >> (uint32 b) const
{
    return _mm512_srli_epi32(v, b);
}

const Vec16ui Vec16ui::Min(const Vec16ui& a, const Vec16ui& b)
{
    return _mm512_min_epu32(a, b);
}

const Vec16ui Vec16ui::Max(const Vec16ui& a, const Vec16ui& b)
{
    return _mm512_max_epu32(a, b);
}


} // namespace Math
} // namespace NFE
