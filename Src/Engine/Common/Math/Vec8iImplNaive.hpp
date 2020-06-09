#pragma once

#include "Vec8f.hpp"

namespace NFE {
namespace Math {

VecBool8i::VecBool8i(const VecBool8f& other)
    : VecBool8i(
        other.Get<0>(), other.Get<1>(), other.Get<2>(), other.Get<3>(),
        other.Get<4>(), other.Get<5>(), other.Get<6>(), other.Get<7>())
{}

VecBool8i::VecBool8i(const VecBool4i& low, const VecBool4i& high)
    : low(low)
    , high(high)
{}

VecBool8i::VecBool8i(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
    : low(e0, e1, e2, e3)
    , high(e4, e5, e6, e7)
{}

template<uint32 index>
bool VecBool8i::Get() const
{
    static_assert(index < 8u, "Invalid index");

    if constexpr (index < 4)
    {
        return low.Get<index>();
    }
    else
    {
        return high.Get<index - 4u>();
    }
}

bool VecBool8i::All() const
{
    return low.All() && high.All();
}

bool VecBool8i::None() const
{
    return low.None() && high.None();
}

bool VecBool8i::Any() const
{
    return low.Any() || high.Any();
}

const VecBool8i VecBool8i::operator & (const VecBool8i rhs) const
{
    return VecBool8i{ low & rhs.low, high & rhs.high };
}

const VecBool8i VecBool8i::operator | (const VecBool8i rhs) const
{
    return VecBool8i{ low | rhs.low, high | rhs.high };
}

const VecBool8i VecBool8i::operator ^ (const VecBool8i rhs) const
{
    return VecBool8i{ low ^ rhs.low, high ^ rhs.high };
}

bool VecBool8i::operator == (const VecBool8i& other) const
{
    return low == other.low && high == other.high;
}

bool VecBool8i::operator != (const VecBool8i& other) const
{
    return low != other.low || high != other.high;
}

///

const Vec8i Vec8i::Zero()
{
    return Vec8i{ 0 };
}

Vec8i::Vec8i(const Vec8i& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec8i::Vec8i(const Vec8ui& other)
    : low{ Vec4i{ other.low } }
    , high{ Vec4i{ other.high } }
{}

Vec8i& Vec8i::operator = (const Vec8i& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

Vec8i::Vec8i(const Vec4i& lo, const Vec4i& hi)
    : low{ lo }, high{ hi }
{}

const Vec8i Vec8i::Cast(const Vec8f& v)
{
    return reinterpret_cast<const Vec8i&>(v);
}

const Vec8f Vec8i::AsVec8f() const
{
    return reinterpret_cast<const Vec8f&>(*this);
}

Vec8i::Vec8i(const int32 e0, const int32 e1, const int32 e2, const int32 e3, const int32 e4, const int32 e5, const int32 e6, const int32 e7)
    : low{ e0, e1, e2, e3 }
    , high{ e4, e5, e6, e7 }
{}

Vec8i::Vec8i(const int32 scalar)
    : low{ scalar }
    , high{ scalar }
{}

Vec8i::Vec8i(const int32* scalarPtr)
    : low{ scalarPtr }
    , high{ scalarPtr + 4 }
{}

const Vec8i Vec8i::Iota(const int32 value)
{
    return Vec8i(value) + Vec8i(0, 1, 2, 3, 4, 5, 6, 7);
}

const Vec8i Vec8i::operator & (const Vec8i& b) const
{
    return { low & b.low, high & b.high };
}

const Vec8i Vec8i::operator | (const Vec8i& b) const
{
    return { low | b.low, high | b.high };
}

const Vec8i Vec8i::operator ^ (const Vec8i& b) const
{
    return { low ^ b.low, high ^ b.high };
}

Vec8i& Vec8i::operator &= (const Vec8i& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

Vec8i& Vec8i::operator |= (const Vec8i& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

Vec8i& Vec8i::operator ^= (const Vec8i& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const Vec8i Vec8i::Select(const Vec8i& a, const Vec8i& b, const VecBool8i& sel)
{
    return { Vec4i::Select(a.low, b.low, sel.low), Vec4i::Select(a.high, b.high, sel.high) };
}

const Vec8i Vec8i::Convert(const Vec8f& v)
{
    return { Vec4i::Convert(v.Low()), Vec4i::Convert(v.High()) };
}

const Vec8f Vec8i::ConvertToVec8f() const
{
    return { low.ConvertToVec4f(), high.ConvertToVec4f() };
}

const Vec8i Vec8i::operator - () const
{
    return Vec8i::Zero() - (*this);
}

const Vec8i Vec8i::operator + (const Vec8i& b) const
{
    return { low + b.low, high + b.high };
}

const Vec8i Vec8i::operator - (const Vec8i& b) const
{
    return { low - b.low, high - b.high };
}

const Vec8i Vec8i::operator * (const Vec8i& b) const
{
    return { low * b.low, high * b.high };
}

Vec8i& Vec8i::operator += (const Vec8i& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

Vec8i& Vec8i::operator -= (const Vec8i& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

Vec8i& Vec8i::operator *= (const Vec8i& b)
{
    low *= b.low;
    high *= b.high;
    return *this;
}

const Vec8i Vec8i::operator + (int32 b) const
{
    return { low + b, high + b };
}

const Vec8i Vec8i::operator - (int32 b) const
{
    return { low - b, high - b };
}

const Vec8i Vec8i::operator * (int32 b) const
{
    return { low * b, high * b };
}

Vec8i& Vec8i::operator += (int32 b)
{
    low += b;
    high += b;
    return *this;
}

Vec8i& Vec8i::operator -= (int32 b)
{
    low -= b;
    high -= b;
    return *this;
}

Vec8i& Vec8i::operator *= (int32 b)
{
    low *= b;
    high *= b;
    return *this;
}

const VecBool8i Vec8i::operator == (const Vec8i& b) const
{
    return VecBool8i{ low == b.low, high == b.high };
}

const VecBool8i Vec8i::operator != (const Vec8i& b) const
{
    return VecBool8i{ low != b.low, high != b.high };
}

const Vec8i Vec8i::operator << (const Vec8i& b) const
{
    return { low << b.low, high << b.high };
}

const Vec8i Vec8i::operator >> (const Vec8i& b) const
{
    return { low >> b.low, high >> b.high };
}

const Vec8i Vec8i::operator << (int32 b) const
{
    return { low << b, high << b };
}

const Vec8i Vec8i::operator >> (int32 b) const
{
    return { low >> b, high >> b };
}

const Vec8i Vec8i::Min(const Vec8i& a, const Vec8i& b)
{
    return { Vec4i::Min(a.low, b.low), Vec4i::Min(a.high, b.high) };
}

const Vec8i Vec8i::Max(const Vec8i& a, const Vec8i& b)
{
    return { Vec4i::Max(a.low, b.low), Vec4i::Max(a.high, b.high) };
}

///

const Vec8ui Vec8ui::Zero()
{
    return Vec8ui{ 0u };
}

Vec8ui::Vec8ui(const Vec8ui& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec8ui::Vec8ui(const Vec8i& other)
    : low{ Vec4ui{ other.low } }
    , high{ Vec4ui{ other.high } }
{}

Vec8ui& Vec8ui::operator = (const Vec8ui& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

Vec8ui::Vec8ui(const Vec4ui& lo, const Vec4ui& hi)
    : low{ lo }, high{ hi }
{}

const Vec8ui Vec8ui::Cast(const Vec8f& v)
{
    return reinterpret_cast<const Vec8ui&>(v);
}

const Vec8f Vec8ui::AsVec8f() const
{
    return reinterpret_cast<const Vec8f&>(*this);
}

Vec8ui::Vec8ui(const uint32 e0, const uint32 e1, const uint32 e2, const uint32 e3, const uint32 e4, const uint32 e5, const uint32 e6, const uint32 e7)
    : low{ e0, e1, e2, e3 }
    , high{ e4, e5, e6, e7 }
{}

Vec8ui::Vec8ui(const uint32 scalar)
    : low{ scalar }
    , high{ scalar }
{}

Vec8ui::Vec8ui(const uint32* scalarPtr)
    : low{ scalarPtr }
    , high{ scalarPtr + 4 }
{}

const Vec8ui Vec8ui::Iota(const uint32 value)
{
    return Vec8ui(value) + Vec8ui(0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u);
}

const Vec8ui Vec8ui::operator & (const Vec8ui& b) const
{
    return { low & b.low, high & b.high };
}

const Vec8ui Vec8ui::operator | (const Vec8ui& b) const
{
    return { low | b.low, high | b.high };
}

const Vec8ui Vec8ui::operator ^ (const Vec8ui& b) const
{
    return { low ^ b.low, high ^ b.high };
}

Vec8ui& Vec8ui::operator &= (const Vec8ui& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

Vec8ui& Vec8ui::operator |= (const Vec8ui& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

Vec8ui& Vec8ui::operator ^= (const Vec8ui& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const Vec8ui Vec8ui::Select(const Vec8ui& a, const Vec8ui& b, const VecBool8i& sel)
{
    return { Vec4ui::Select(a.low, b.low, sel.low), Vec4ui::Select(a.high, b.high, sel.high) };
}

const Vec8ui Vec8ui::Convert(const Vec8f& v)
{
    return { Vec4ui::Convert(v.Low()), Vec4ui::Convert(v.High()) };
}

const Vec8f Vec8ui::ConvertToVec8f() const
{
    return { low.ConvertToVec4f(), high.ConvertToVec4f() };
}

const Vec8ui Vec8ui::operator + (const Vec8ui& b) const
{
    return { low + b.low, high + b.high };
}

const Vec8ui Vec8ui::operator - (const Vec8ui& b) const
{
    return { low - b.low, high - b.high };
}

Vec8ui& Vec8ui::operator += (const Vec8ui& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

Vec8ui& Vec8ui::operator -= (const Vec8ui& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

const Vec8ui Vec8ui::operator + (int32 b) const
{
    return { low + b, high + b };
}

const Vec8ui Vec8ui::operator - (int32 b) const
{
    return { low - b, high - b };
}

Vec8ui& Vec8ui::operator += (int32 b)
{
    low += b;
    high += b;
    return *this;
}

Vec8ui& Vec8ui::operator -= (int32 b)
{
    low -= b;
    high -= b;
    return *this;
}

const VecBool8i Vec8ui::operator == (const Vec8ui& b) const
{
    return VecBool8i{ low == b.low, high == b.high };
}

const VecBool8i Vec8ui::operator != (const Vec8ui& b) const
{
    return VecBool8i{ low != b.low, high != b.high };
}

const Vec8ui Vec8ui::operator << (const Vec8ui& b) const
{
    return { low << b.low, high << b.high };
}

const Vec8ui Vec8ui::operator >> (const Vec8ui& b) const
{
    return { low >> b.low, high >> b.high };
}

const Vec8ui Vec8ui::operator << (int32 b) const
{
    return { low << b, high << b };
}

const Vec8ui Vec8ui::operator >> (int32 b) const
{
    return { low >> b, high >> b };
}

const Vec8ui Vec8ui::Min(const Vec8ui& a, const Vec8ui& b)
{
    return { Vec4ui::Min(a.low, b.low), Vec4ui::Min(a.high, b.high) };
}

const Vec8ui Vec8ui::Max(const Vec8ui& a, const Vec8ui& b)
{
    return { Vec4ui::Max(a.low, b.low), Vec4ui::Max(a.high, b.high) };
}

///

const Vec8f Gather8(const float* basePtr, const Vec8i& indices)
{
    Vec8f result;
    for (uint32 i = 0; i < 8; ++i)
    {
        result[i] = basePtr[indices[i]];
    }
    return result;
}

} // namespace Math
} // namespace NFE
