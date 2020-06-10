#pragma once

namespace NFE {
namespace Math {

VecBool16f::VecBool16f(bool scalar)
    : low(scalar)
    , high(scalar)
{}

VecBool16f::VecBool16f(const VecBool8f& low, const VecBool8f& high)
    : low(low)
    , high(high)
{}

VecBool16f::VecBool16f(
    bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7,
    bool e8, bool e9, bool e10, bool e11, bool e12, bool e13, bool e14, bool e15)
    : low{ e0, e1, e2, e3, e4, e5, e6, e7 }
    , high{ e8, e9, e10, e11, e12, e13, e14, e15 }
{}

template<uint32 index>
bool VecBool16f::Get() const
{
    static_assert(index < 16u, "Invalid index");

    if constexpr (index < 8)
    {
        return low.Get<index>();
    }
    else
    {
        return high.Get<index - 8>();
    }
}

uint32 VecBool16f::GetMask() const
{
    return low.GetMask() | (high.GetMask() << 8u);
}

bool VecBool16f::All() const
{
    return low.All() && high.All();
}

bool VecBool16f::None() const
{
    return low.None() && high.None();
}

bool VecBool16f::Any() const
{
    return low.Any() || high.Any();
}

const VecBool16f VecBool16f::operator & (const VecBool16f rhs) const
{
    return VecBool16f{ low & rhs.low, high & rhs.high };
}

const VecBool16f VecBool16f::operator | (const VecBool16f rhs) const
{
    return VecBool16f{ low | rhs.low, high | rhs.high };
}

const VecBool16f VecBool16f::operator ^ (const VecBool16f rhs) const
{
    return VecBool16f{ low ^ rhs.low, high ^ rhs.high };
}

bool VecBool16f::operator == (const VecBool16f rhs) const
{
    return (low == rhs.low) && (high == rhs.high);
}


Vec16f::Vec16f()
    : low{}
    , high{}
{}

Vec16f::Vec16f(const Vec16f& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec16f::Vec16f(const Vec8f& lo, const Vec8f& hi)
    : low{ lo }
    , high{ hi }
{}

Vec16f::Vec16f(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7, float e8, float e9, float e10, float e11, float e12, float e13, float e14, float e15)
    : low{ e0, e1, e2, e3, e4, e5, e6, e7 }
    , high{ e8, e9, e10, e11, e12, e13, e14, e15 }
{}

Vec16f::Vec16f(const float* src)
    : low{ src }
    , high{ src + 8 }
{}

Vec16f::Vec16f(const float scalar)
    : low{ scalar }
    , high{ scalar }
{}

Vec16f::Vec16f(const int32 i)
    : low{ i }
    , high{ i }
{}

Vec16f::Vec16f(const uint32 u)
    : low{ u }
    , high{ u }
{}

const Vec16f Vec16f::Zero()
{
    return { Vec8f::Zero(), Vec8f::Zero() };
}

const Vec16f Vec16f::FromInteger(int32 x)
{
    return { Vec8f::FromInteger(x), Vec8f::FromInteger(x) };
}

Vec16f& Vec16f::operator = (const Vec16f& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

const Vec16f Vec16f::Select(const Vec16f& a, const Vec16f& b, const VecBool16f& sel)
{
    return { Vec8f::Select(a.low, b.low, sel.low), Vec8f::Select(a.high, b.high, sel.high) };
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vec16f Vec16f::Select(const Vec16f& a, const Vec16f& b)
{
    return { Vec8f::Select<selX,selY,selZ,selW>(a.low, b.low), Vec8f::Select<selX,selY,selZ,selW>(a.high, b.high) };
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec16f Vec16f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { low.Swizzle<ix, iy, iz, iw>(), high.Swizzle<ix, iy, iz, iw>() };
}

const Vec8f Vec16f::Low() const
{
    return low;
}

const Vec8f Vec16f::High() const
{
    return high;
}

const Vec16f Vec16f::operator & (const Vec16f& b) const
{
    return { low & b.low, high & b.high };
}

const Vec16f Vec16f::operator | (const Vec16f& b) const
{
    return { low | b.low, high | b.high };
}

const Vec16f Vec16f::operator ^ (const Vec16f& b) const
{
    return { low ^ b.low, high ^ b.high };
}

Vec16f& Vec16f::operator &= (const Vec16f& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

Vec16f& Vec16f::operator |= (const Vec16f& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

Vec16f& Vec16f::operator ^= (const Vec16f& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const Vec16f Vec16f::operator- () const
{
    return { -low, -high };
}

const Vec16f Vec16f::operator + (const Vec16f& b) const
{
    return { low + b.low, high + b.high };
}

const Vec16f Vec16f::operator - (const Vec16f& b) const
{
    return { low - b.low, high - b.high };
}

const Vec16f Vec16f::operator * (const Vec16f& b) const
{
    return { low * b.low, high * b.high };
}

const Vec16f Vec16f::operator / (const Vec16f& b) const
{
    return { low / b.low, high / b.high };
}

const Vec16f Vec16f::operator * (float b) const
{
    return { low * b, high * b };
}

const Vec16f Vec16f::operator / (float b) const
{
    return { low / b, high / b };
}

const Vec16f operator * (float a, const Vec16f& b)
{
    return { a * b.low, a * b.high };
}

Vec16f& Vec16f::operator += (const Vec16f& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

Vec16f& Vec16f::operator -= (const Vec16f& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

Vec16f& Vec16f::operator *= (const Vec16f& b)
{
    low *= b.low;
    high *= b.high;
    return *this;
}

Vec16f& Vec16f::operator /= (const Vec16f& b)
{
    low /= b.low;
    high /= b.high;
    return *this;
}

Vec16f& Vec16f::operator *= (float b)
{
    low *= b;
    high *= b;
    return *this;
}

Vec16f& Vec16f::operator/= (float b)
{
    low /= b;
    high /= b;
    return *this;
}

const Vec16f Vec16f::MulAndAdd(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
    return { Vec8f::MulAndAdd(a.low, b.low, c.low), Vec8f::MulAndAdd(a.high, b.high, c.high) };
}

const Vec16f Vec16f::MulAndSub(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
    return { Vec8f::MulAndSub(a.low, b.low, c.low), Vec8f::MulAndSub(a.high, b.high, c.high) };
}

const Vec16f Vec16f::NegMulAndAdd(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
    return { Vec8f::NegMulAndAdd(a.low, b.low, c.low), Vec8f::NegMulAndAdd(a.high, b.high, c.high) };
}

const Vec16f Vec16f::NegMulAndSub(const Vec16f& a, const Vec16f& b, const Vec16f& c)
{
    return { Vec8f::NegMulAndSub(a.low, b.low, c.low), Vec8f::NegMulAndSub(a.high, b.high, c.high) };
}

const Vec16f Vec16f::Floor(const Vec16f& v)
{
    return { Vec8f::Floor(v.low), Vec8f::Floor(v.high) };
}

const Vec16f Vec16f::Sqrt(const Vec16f& v)
{
    return { Vec8f::Sqrt(v.low), Vec8f::Sqrt(v.high) };
}

const Vec16f Vec16f::Reciprocal(const Vec16f& v)
{
    return { Vec8f::Reciprocal(v.low), Vec8f::Reciprocal(v.high) };
}

const Vec16f Vec16f::FastReciprocal(const Vec16f& v)
{
    return { Vec8f::FastReciprocal(v.low), Vec8f::FastReciprocal(v.high) };
}

const Vec16f Vec16f::Min(const Vec16f& a, const Vec16f& b)
{
    return { Vec8f::Min(a.low, b.low), Vec8f::Min(a.high, b.high) };
}

const Vec16f Vec16f::Max(const Vec16f& a, const Vec16f& b)
{
    return { Vec8f::Max(a.low, b.low), Vec8f::Max(a.high, b.high) };
}

const Vec16f Vec16f::Abs(const Vec16f& v)
{
    return { Vec8f::Abs(v.low), Vec8f::Abs(v.high) };
}

uint32 Vec16f::GetSignMask() const
{
    return low.GetSignMask() | (high.GetSignMask() << 8u);
}

const VecBool16f Vec16f::operator == (const Vec16f& b) const
{
    return { low == b.low, high == b.high };
}

const VecBool16f Vec16f::operator < (const Vec16f& b) const
{
    return { low < b.low, high < b.high };
}

const VecBool16f Vec16f::operator <= (const Vec16f& b) const
{
    return { low <= b.low, high <= b.high };
}

const VecBool16f Vec16f::operator > (const Vec16f& b) const
{
    return { low > b.low, high > b.high };
}

const VecBool16f Vec16f::operator >= (const Vec16f& b) const
{
    return { low >= b.low, high >= b.high };
}

const VecBool16f Vec16f::operator != (const Vec16f& b) const
{
    return { low != b.low, high != b.high };
}

bool Vec16f::IsZero() const
{
    return low.IsZero() && high.IsZero();
}

bool Vec16f::IsNaN() const
{
    return low.IsNaN() || high.IsNaN();
}

bool Vec16f::IsInfinite() const
{
    return low.IsInfinite() || high.IsInfinite();
}

} // namespace Math
} // namespace NFE
