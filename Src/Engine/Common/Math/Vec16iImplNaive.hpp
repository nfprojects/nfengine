#pragma once

#include "Vec16i.hpp"
#include "Vec16f.hpp"

namespace NFE {
namespace Math {

const Vec16i Vec16i::Zero()
{
    return Vec16i{ 0 };
}

Vec16i::Vec16i(const Vec16i& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec16i::Vec16i(const Vec16ui& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec16i::Vec16i(const Vec8i& low, const Vec8i& high)
    : low{ low }
    , high{ high }
{}

Vec16i& Vec16i::operator = (const Vec16i& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

const Vec16i Vec16i::Cast(const Vec16f& v)
{
    return reinterpret_cast<const Vec16i&>(v);
}

const Vec16f Vec16i::AsVec16f() const
{
    return reinterpret_cast<const Vec16f&>(*this);
}

Vec16i::Vec16i(
    const int32 e0, const int32 e1, const int32 e2, const int32 e3,
    const int32 e4, const int32 e5, const int32 e6, const int32 e7,
    const int32 e8, const int32 e9, const int32 e10, const int32 e11,
    const int32 e12, const int32 e13, const int32 e14, const int32 e15
)
    : low{ e0, e1, e2, e3, e4, e5, e6, e7 }
    , high{ e8, e9, e10, e11, e12, e13, e14, e15 }
{}

Vec16i::Vec16i(const int32 scalar)
    : low{ scalar }
    , high{ scalar }
{}

Vec16i::Vec16i(const int32* scalarPtr)
    : low{ scalarPtr }
    , high{ scalarPtr + 8 }
{}

const Vec16i Vec16i::Iota(const int32 value)
{
    return Vec16i(value) + Vec16i(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
}

const Vec16i Vec16i::Select(const Vec16i& a, const Vec16i& b, const VecBool16& sel)
{
    return { Vec8i::Select(a.low, b.low, sel.low), Vec8i::Select(a.high, b.high, sel.high) };
}

const Vec16i Vec16i::operator & (const Vec16i& b) const
{
    return { low & b.low, high & b.high };
}

const Vec16i Vec16i::operator | (const Vec16i& b) const
{
    return { low | b.low, high | b.high };
}

const Vec16i Vec16i::operator ^ (const Vec16i& b) const
{
    return { low ^ b.low, high ^ b.high };
}

Vec16i& Vec16i::operator &= (const Vec16i& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

Vec16i& Vec16i::operator |= (const Vec16i& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

Vec16i& Vec16i::operator ^= (const Vec16i& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const Vec16i Vec16i::Convert(const Vec16f& v)
{
    return { Vec8i::Convert(v.low), Vec8i::Convert(v.high) };
}

const Vec16f Vec16i::ConvertToVec16f() const
{
    return { low.ConvertToVec8f(), high.ConvertToVec8f() };
}

const Vec16i Vec16i::operator - () const
{
    return Vec16i::Zero() - (*this);
}

const Vec16i Vec16i::operator + (const Vec16i& b) const
{
    return { low + b.low, high + b.high };
}

const Vec16i Vec16i::operator - (const Vec16i& b) const
{
    return { low - b.low, high - b.high };
}

const Vec16i Vec16i::operator * (const Vec16i& b) const
{
    return { low * b.low, high * b.high };
}

Vec16i& Vec16i::operator += (const Vec16i& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

Vec16i& Vec16i::operator -= (const Vec16i& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

Vec16i& Vec16i::operator *= (const Vec16i& b)
{
    low *= b.low;
    high *= b.high;
    return *this;
}

const Vec16i Vec16i::operator + (int32 b) const
{
    return { low + b, high + b };
}

const Vec16i Vec16i::operator - (int32 b) const
{
    return { low - b, high - b };
}

const Vec16i Vec16i::operator * (int32 b) const
{
    return { low * b, high * b };
}

Vec16i& Vec16i::operator += (int32 b)
{
    low += b;
    high += b;
    return *this;
}

Vec16i& Vec16i::operator -= (int32 b)
{
    low -= b;
    high -= b;
    return *this;
}

Vec16i& Vec16i::operator *= (int32 b)
{
    low *= b;
    high *= b;
    return *this;
}

const VecBool16 Vec16i::operator == (const Vec16i& b) const
{
    return { low == b.low, high == b.high };
}

const VecBool16 Vec16i::operator != (const Vec16i& b) const
{
    return { low != b.low, high != b.high };
}

const Vec16i Vec16i::operator << (const Vec16i& b) const
{
    return { low << b.low, high << b.high };
}

const Vec16i Vec16i::operator >> (const Vec16i& b) const
{
    return { low >> b.low, high >> b.high };
}

const Vec16i Vec16i::operator << (int32 b) const
{
    return { low << b, high << b };
}

const Vec16i Vec16i::operator >> (int32 b) const
{
    return { low >> b, high >> b };
}

const Vec16i Vec16i::Min(const Vec16i& a, const Vec16i& b)
{
    return { Vec8i::Min(a.low, b.low), Vec8i::Min(a.high, b.high) };
}

const Vec16i Vec16i::Max(const Vec16i& a, const Vec16i& b)
{
    return { Vec8i::Max(a.low, b.low), Vec8i::Max(a.high, b.high) };
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const Vec16ui Vec16ui::Zero()
{
    return Vec16ui{ 0u };
}

Vec16ui::Vec16ui(const Vec16i& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec16ui::Vec16ui(const Vec16ui& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec16ui::Vec16ui(const Vec8ui& low, const Vec8ui& high)
    : low{ low }
    , high{ high }
{}

Vec16ui& Vec16ui::operator = (const Vec16ui& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

const Vec16ui Vec16ui::Cast(const Vec16f& v)
{
    return reinterpret_cast<const Vec16ui&>(v);
}

const Vec16f Vec16ui::AsVec16f() const
{
    return reinterpret_cast<const Vec16f&>(*this);
}

Vec16ui::Vec16ui(
    const uint32 e0, const uint32 e1, const uint32 e2, const uint32 e3,
    const uint32 e4, const uint32 e5, const uint32 e6, const uint32 e7,
    const uint32 e8, const uint32 e9, const uint32 e10, const uint32 e11,
    const uint32 e12, const uint32 e13, const uint32 e14, const uint32 e15
)
    : low{ e0, e1, e2, e3, e4, e5, e6, e7 }
    , high{ e8, e9, e10, e11, e12, e13, e14, e15 }
{}

Vec16ui::Vec16ui(const uint32 scalar)
    : low{ scalar }
    , high{ scalar }
{}

Vec16ui::Vec16ui(const uint32* scalarPtr)
    : low{ scalarPtr }
    , high{ scalarPtr + 8 }
{}

const Vec16ui Vec16ui::Iota(const uint32 value)
{
    return Vec16ui(value) + Vec16ui(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
}

const Vec16ui Vec16ui::Select(const Vec16ui& a, const Vec16ui& b, const VecBool16& sel)
{
    return { Vec8ui::Select(a.low, b.low, sel.low), Vec8ui::Select(a.high, b.high, sel.high) };
}

const Vec16ui Vec16ui::operator & (const Vec16ui& b) const
{
    return { low & b.low, high & b.high };
}

const Vec16ui Vec16ui::operator | (const Vec16ui& b) const
{
    return { low | b.low, high | b.high };
}

const Vec16ui Vec16ui::operator ^ (const Vec16ui& b) const
{
    return { low ^ b.low, high ^ b.high };

}

Vec16ui& Vec16ui::operator &= (const Vec16ui& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

Vec16ui& Vec16ui::operator |= (const Vec16ui& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

Vec16ui& Vec16ui::operator ^= (const Vec16ui& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const Vec16ui Vec16ui::operator + (const Vec16ui& b) const
{
    return { low + b.low, high + b.high };
}

const Vec16ui Vec16ui::operator - (const Vec16ui& b) const
{
    return { low - b.low, high - b.high };
}

Vec16ui& Vec16ui::operator += (const Vec16ui& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

Vec16ui& Vec16ui::operator -= (const Vec16ui& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

const Vec16ui Vec16ui::operator + (uint32 b) const
{
    return { low + b, high + b };
}

const Vec16ui Vec16ui::operator - (uint32 b) const
{
    return { low - b, high - b };
}

Vec16ui& Vec16ui::operator += (uint32 b)
{
    low += b;
    high += b;
    return *this;
}

Vec16ui& Vec16ui::operator -= (uint32 b)
{
    low -= b;
    high -= b;
    return *this;
}

const VecBool16 Vec16ui::operator == (const Vec16ui& b) const
{
    return { low == b.low, high == b.high };
}

const VecBool16 Vec16ui::operator != (const Vec16ui& b) const
{
    return { low != b.low, high != b.high };
}

const Vec16ui Vec16ui::operator << (const Vec16ui& b) const
{
    return { low << b.low, high << b.high };
}

const Vec16ui Vec16ui::operator >> (const Vec16ui& b) const
{
    return { low >> b.low, high >> b.high };
}

const Vec16ui Vec16ui::operator << (uint32 b) const
{
    return { low << b, high << b };
}

const Vec16ui Vec16ui::operator >> (uint32 b) const
{
    return { low >> b, high >> b };
}

const Vec16ui Vec16ui::Min(const Vec16ui& a, const Vec16ui& b)
{
    return { Vec8ui::Min(a.low, b.low), Vec8ui::Min(a.high, b.high) };
}

const Vec16ui Vec16ui::Max(const Vec16ui& a, const Vec16ui& b)
{
    return { Vec8ui::Max(a.low, b.low), Vec8ui::Max(a.high, b.high) };
}


} // namespace Math
} // namespace NFE
