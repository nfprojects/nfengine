#pragma once

#include "Vec8f.hpp"

namespace NFE {
namespace Math {

const Vec8i Vec8i::Zero()
{
    return Vec8i{ 0 };
}

Vec8i::Vec8i(const Vec8i& other)
    : low{ other.low }
    , high{ other.high }
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
    : i{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

Vec8i::Vec8i(const int32 i)
    : low{ i }
    , high{ i }
{}

Vec8i::Vec8i(const uint32 u)
    : low{ u }
    , high{ u }
{}

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

const Vec8i Vec8i::Convert(const Vec8f& v)
{
    return { Vec4i::Convert(v.low), Vec4i::Convert(v.high) };
}

const Vec8f Vec8i::ConvertToVec8f() const
{
    return { low.ConvertToFloat(), high.ConvertToFloat() };
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

const Vec8i Vec8i::operator % (int32 b) const
{
    // TODO
    return Vec8i(i[0] % b, i[1] % b, i[2] % b, i[3] % b, i[4] % b, i[5] % b, i[6] % b, i[7] % b);
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

bool Vec8i::operator == (const Vec8i& b) const
{
    return (low == b.low).All() && (high == b.high).All();
}

bool Vec8i::operator != (const Vec8i& b) const
{
    return (low != b.low).Any() || (high != b.high).Any();
}

const Vec8i Vec8i::operator << (const Vec8i& b) const
{
    return
    {
        i[0] << b.i[0],
        i[1] << b.i[1],
        i[2] << b.i[2],
        i[3] << b.i[3],
        i[4] << b.i[4],
        i[5] << b.i[5],
        i[6] << b.i[6],
        i[7] << b.i[7]
    };
}

const Vec8i Vec8i::operator >> (const Vec8i& b) const
{
    return
    {
        i[0] >> b.i[0],
        i[1] >> b.i[1],
        i[2] >> b.i[2],
        i[3] >> b.i[3],
        i[4] >> b.i[4],
        i[5] >> b.i[5],
        i[6] >> b.i[6],
        i[7] >> b.i[7]
    };
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
