#pragma once

namespace NFE {
namespace Math {

VecBool8f::VecBool8f(bool e0, bool e1, bool e2, bool e3, bool e4, bool e5, bool e6, bool e7)
    : b{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

VecBool8f::VecBool8f(const VecBool4f& low, const VecBool4f& high)
    : b{ low.Get<0>(), low.Get<1>(), low.Get<2>(), low.Get<3>(),
         high.Get<0>(), high.Get<1>(), high.Get<2>(), high.Get<3>() }
{}

template<uint32 index>
bool VecBool8f::Get() const
{
    static_assert(index < 8, "Invalid index");
    return b[index];
}

int VecBool8f::GetMask() const
{
    int32 ret = 0;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret |= b[i] ? (1 << i) : 0;
    }
    return ret;
}

bool VecBool8f::All() const
{
    bool ret = true;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret &= b[i];
    }
    return ret;
}

bool VecBool8f::None() const
{
    bool ret = true;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret &= !(b[i]);
    }
    return ret;
}

bool VecBool8f::Any() const
{
    bool ret = false;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret |= (b[i]);
    }
    return ret;
}

const VecBool8f VecBool8f::operator & (const VecBool8f rhs) const
{
    VecBool8f ret;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret.b[i] = b[i] && rhs.b[i];
    }
    return ret;
}

const VecBool8f VecBool8f::operator | (const VecBool8f rhs) const
{
    VecBool8f ret;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret.b[i] = b[i] || rhs.b[i];
    }
    return ret;
}

const VecBool8f VecBool8f::operator ^ (const VecBool8f rhs) const
{
    VecBool8f ret;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret.b[i] = b[i] ^ rhs.b[i];
    }
    return ret;
}

bool VecBool8f::operator == (const VecBool8f rhs) const
{
    bool ret = true;
    for (uint32 i = 0; i < 8; ++i)
    {
        ret &= b[i] == rhs.b[i];
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Vec8f::Vec8f()
    : low{}
    , high{}
{}

Vec8f::Vec8f(const Vec8f& other)
    : low{ other.low }
    , high{ other.high }
{}

Vec8f::Vec8f(const Vec4f& lo)
    : low{ lo }
    , high{ Vec4f::Zero() }
{}

Vec8f::Vec8f(const Vec4f& lo, const Vec4f& hi)
    : low{ lo }
    , high{ hi }
{}

Vec8f::Vec8f(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7)
    : f{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

Vec8f::Vec8f(int32 e0, int32 e1, int32 e2, int32 e3, int32 e4, int32 e5, int32 e6, int32 e7)
    : i{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

Vec8f::Vec8f(uint32 e0, uint32 e1, uint32 e2, uint32 e3, uint32 e4, uint32 e5, uint32 e6, uint32 e7)
    : u{ e0, e1, e2, e3, e4, e5, e6, e7 }
{}

Vec8f::Vec8f(const float* src)
    : low{ src }
    , high{ src + 4 }
{}

Vec8f::Vec8f(const float scalar)
    : low{ scalar }
    , high{ scalar }
{}

Vec8f::Vec8f(const int32 i)
    : low{ i }
    , high{ i }
{}

Vec8f::Vec8f(const uint32 u)
    : low{ u }
    , high{ u }
{}

const Vec8f Vec8f::Zero()
{
    return { Vec4f::Zero(), Vec4f::Zero() };
}

const Vec8f Vec8f::FromInteger(int32 x)
{
    return { Vec4f::FromInteger(x), Vec4f::FromInteger(x) };
}

Vec8f& Vec8f::operator = (const Vec8f& other)
{
    low = other.low;
    high = other.high;
    return *this;
}

const Vec8f Vec8f::Select(const Vec8f& a, const Vec8f& b, const VecBool8f& sel)
{
    return
    {
        sel.Get<0>() ? b.f[0] : a.f[0],
        sel.Get<1>() ? b.f[1] : a.f[1],
        sel.Get<2>() ? b.f[2] : a.f[2],
        sel.Get<3>() ? b.f[3] : a.f[3],
        sel.Get<4>() ? b.f[3] : a.f[4],
        sel.Get<5>() ? b.f[4] : a.f[5],
        sel.Get<6>() ? b.f[5] : a.f[6],
        sel.Get<7>() ? b.f[6] : a.f[7],
    };
}

template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
const Vec8f Vec8f::Select(const Vec8f& a, const Vec8f& b)
{
    return { Vec4f::Select<selX,selY,selZ,selW>(a.low, b.low), Vec4f::Select<selX,selY,selZ,selW>(a.high, b.high) };
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
const Vec8f Vec8f::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return { low.Swizzle<ix, iy, iz, iw>(), high.Swizzle<ix, iy, iz, iw>() };
}

const Vec4f Vec8f::Low() const
{
    return low;
}

const Vec4f Vec8f::High() const
{
    return high;
}

const Vec8f Vec8f::operator & (const Vec8f& b) const
{
    return { low & b.low, high & b.high };
}

const Vec8f Vec8f::operator | (const Vec8f& b) const
{
    return { low | b.low, high | b.high };
}

const Vec8f Vec8f::operator ^ (const Vec8f& b) const
{
    return { low ^ b.low, high ^ b.high };
}

Vec8f& Vec8f::operator &= (const Vec8f& b)
{
    low &= b.low;
    high &= b.high;
    return *this;
}

Vec8f& Vec8f::operator |= (const Vec8f& b)
{
    low |= b.low;
    high |= b.high;
    return *this;
}

Vec8f& Vec8f::operator ^= (const Vec8f& b)
{
    low ^= b.low;
    high ^= b.high;
    return *this;
}

const Vec8f Vec8f::operator- () const
{
    return { -low, -high };
}

const Vec8f Vec8f::operator + (const Vec8f& b) const
{
    return { low + b.low, high + b.high };
}

const Vec8f Vec8f::operator - (const Vec8f& b) const
{
    return { low - b.low, high - b.high };
}

const Vec8f Vec8f::operator * (const Vec8f& b) const
{
    return { low * b.low, high * b.high };
}

const Vec8f Vec8f::operator / (const Vec8f& b) const
{
    return { low / b.low, high / b.high };
}

const Vec8f Vec8f::operator * (float b) const
{
    return { low * b, high * b };
}

const Vec8f Vec8f::operator / (float b) const
{
    return { low / b, high / b };
}

const Vec8f operator * (float a, const Vec8f& b)
{
    return { a * b.low, a * b.high };
}

Vec8f& Vec8f::operator += (const Vec8f& b)
{
    low += b.low;
    high += b.high;
    return *this;
}

Vec8f& Vec8f::operator -= (const Vec8f& b)
{
    low -= b.low;
    high -= b.high;
    return *this;
}

Vec8f& Vec8f::operator *= (const Vec8f& b)
{
    low *= b.low;
    high *= b.high;
    return *this;
}

Vec8f& Vec8f::operator /= (const Vec8f& b)
{
    low /= b.low;
    high /= b.high;
    return *this;
}

Vec8f& Vec8f::operator *= (float b)
{
    low *= b;
    high *= b;
    return *this;
}

Vec8f& Vec8f::operator/= (float b)
{
    low /= b;
    high /= b;
    return *this;
}

const Vec8f Vec8f::MulAndAdd(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
    return a * b + c;
}

const Vec8f Vec8f::MulAndSub(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
    return a * b - c;
}

const Vec8f Vec8f::NegMulAndAdd(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
    return -(a * b) + c;
}

const Vec8f Vec8f::NegMulAndSub(const Vec8f& a, const Vec8f& b, const Vec8f& c)
{
    return c - a * b;
}

const Vec8f Vec8f::Floor(const Vec8f& v)
{
    return { Vec4f::Floor(v.low), Vec4f::Floor(v.high) };
}

const Vec8f Vec8f::Sqrt(const Vec8f& v)
{
    return { Vec4f::Sqrt(v.low), Vec4f::Sqrt(v.high) };
}

const Vec8f Vec8f::Reciprocal(const Vec8f& v)
{
    return { Vec4f::Reciprocal(v.low), Vec4f::Reciprocal(v.high) };
}

const Vec8f Vec8f::FastReciprocal(const Vec8f& v)
{
    return { Vec4f::FastReciprocal(v.low), Vec4f::FastReciprocal(v.high) };
}

const Vec8f Vec8f::Min(const Vec8f& a, const Vec8f& b)
{
    return { Vec4f::Min(a.low, b.low), Vec4f::Min(a.high, b.high) };
}

const Vec8f Vec8f::Max(const Vec8f& a, const Vec8f& b)
{
    return { Vec4f::Max(a.low, b.low), Vec4f::Max(a.high, b.high) };
}

const Vec8f Vec8f::Abs(const Vec8f& v)
{
    return { Vec4f::Abs(v.low), Vec4f::Abs(v.high) };
}

uint32 Vec8f::GetSignMask() const
{
    return low.GetSignMask() | (high.GetSignMask() << 4u);
}

const Vec8f Vec8f::HorizontalMax() const
{
    const Vec4f max = Vec4f::Max(low.HorizontalMax(), high.HorizontalMax());
    return { max, max };
}

const Vec8f Vec8f::Fmod1(const Vec8f& v)
{
    return { Vec4f::Fmod1(v.low), Vec4f::Fmod1(v.high) };
}

/*
void Vec8f::Transpose8x8(Vec8f& v0, Vec8f& v1, Vec8f& v2, Vec8f& v3, Vec8f& v4, Vec8f& v5, Vec8f& v6, Vec8f& v7)
{
    // TODO
}
*/

const VecBool8f Vec8f::operator == (const Vec8f& b) const
{
    return { low == b.low, high == b.high };
}

const VecBool8f Vec8f::operator < (const Vec8f& b) const
{
    return { low < b.low, high < b.high };
}

const VecBool8f Vec8f::operator <= (const Vec8f& b) const
{
    return { low <= b.low, high <= b.high };
}

const VecBool8f Vec8f::operator > (const Vec8f& b) const
{
    return { low > b.low, high > b.high };
}

const VecBool8f Vec8f::operator >= (const Vec8f& b) const
{
    return { low >= b.low, high >= b.high };
}

const VecBool8f Vec8f::operator != (const Vec8f& b) const
{
    return { low != b.low, high != b.high };
}

bool Vec8f::IsZero() const
{
    return low.IsZero().All() && high.IsZero().All();
}

bool Vec8f::IsNaN() const
{
    return low.IsNaN().All() || high.IsNaN().All();
}

bool Vec8f::IsInfinite() const
{
    return low.IsInfinite().All() || high.IsInfinite().All();
}

} // namespace Math
} // namespace NFE
