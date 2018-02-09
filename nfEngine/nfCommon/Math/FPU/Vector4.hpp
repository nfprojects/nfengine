/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU version of Vector4 class definitions.
 */

#pragma once

#if defined(NFE_USE_SSE)
#error "The header cannot be used when NFE_USE_SSE is defined!"
#endif

namespace NFE {
namespace Math {

// Constructors ===================================================================================

Vector4::Vector4()
{
    f[0] = f[1] = f[2] = f[3] = 0.0f;
}

Vector4::Vector4(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w)
{
}

Vector4::Vector4(int x, int y, int z, int w)
{
    i[0] = x;
    i[1] = y;
    i[2] = z;
    i[3] = w;
}

// copy array of 4 floats
Vector4::Vector4(const float* src)
{
    f[0] = src[0];
    f[1] = src[1];
    f[2] = src[2];
    f[3] = src[3];
}

Vector4::Vector4(const Float2& src)
{
    f[0] = src.x;
    f[1] = src.y;
    f[2] = 0.0f;
    f[3] = 0.0f;
}

Vector4::Vector4(const Float3& src)
{
    f[0] = src.x;
    f[1] = src.y;
    f[2] = src.z;
    f[3] = 0.0f;
}

Vector4::Vector4(const Float4& src)
{
    f[0] = src.x;
    f[1] = src.y;
    f[2] = src.z;
    f[3] = src.w;
}

void Vector4::Set(float scalar)
{
    f[0] = scalar;
    f[1] = scalar;
    f[2] = scalar;
    f[3] = scalar;
}

// Load & store ===================================================================================

Vector4 Vector4::Load4(const unsigned char* src)
{
    Vector4 vec;
    vec[0] = static_cast<float>(src[0]);
    vec[1] = static_cast<float>(src[1]);
    vec[2] = static_cast<float>(src[2]);
    vec[3] = static_cast<float>(src[3]);
    return vec;
}

void Vector4::Store4(uint8* dest) const
{
    // TODO: saturate to <0, 255>
    dest[0] = static_cast<unsigned char>(f[0]);
    dest[1] = static_cast<unsigned char>(f[1]);
    dest[2] = static_cast<unsigned char>(f[2]);
    dest[3] = static_cast<unsigned char>(f[3]);
}

void Vector4::Store(float* dest) const
{
    dest[0] = f[0];
    dest[1] = f[1];
    dest[2] = f[2];
    dest[3] = f[3];
}

void Vector4::Store(Float2* dest) const
{
    dest->x = f[0];
    dest->y = f[1];
}

void Vector4::Store(Float3* dest) const
{
    dest->x = f[0];
    dest->y = f[1];
    dest->z = f[2];
}

void Vector4::Store(Float4* dest) const
{
    dest->x = f[0];
    dest->y = f[1];
    dest->z = f[2];
    dest->w = f[3];
}

template<bool negX, bool negY, bool negZ, bool negW>
Vector4 Vector4::ChangeSign() const
{
    return Vector4(
        negX ? -f[0] : f[0],
        negY ? -f[1] : f[1],
        negZ ? -f[2] : f[2],
        negW ? -f[3] : f[3]
    );
}

// Elements rearrangement =========================================================================

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Vector4 Vector4::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return Vector4(f[ix], f[iy], f[iz], f[iw]);
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Vector4 Vector4::Blend(const Vector4& a, const Vector4& b)
{
    static_assert(ix < 2, "Invalid index for X component");
    static_assert(iy < 2, "Invalid index for Y component");
    static_assert(iz < 2, "Invalid index for Z component");
    static_assert(iw < 2, "Invalid index for W component");

    return Vector4(ix == 0 ? a[0] : b[0],
                  iy == 0 ? a[1] : b[1],
                  iz == 0 ? a[2] : b[2],
                  iw == 0 ? a[3] : b[3]);
}

Vector4 Vector4::SplatX() const
{
    return Vector4(f[0], f[0], f[0], f[0]);
}

Vector4 Vector4::SplatY() const
{
    return Vector4(f[1], f[1], f[1], f[1]);
}

Vector4 Vector4::SplatZ() const
{
    return Vector4(f[2], f[2], f[2], f[2]);
}

Vector4 Vector4::SplatW() const
{
    return Vector4(f[3], f[3], f[3], f[3]);
}

Vector4 Vector4::Splat(float f)
{
    return Vector4(f, f, f, f);
}

Vector4 Vector4::SelectBySign(const Vector4& a, const Vector4& b, const Vector4& sel)
{
    Vector4 ret;
    ret[0] = sel[0] > 0.0f ? a[0] : b[0];
    ret[1] = sel[1] > 0.0f ? a[1] : b[1];
    ret[2] = sel[2] > 0.0f ? a[2] : b[2];
    ret[3] = sel[3] > 0.0f ? a[3] : b[3];
    return ret;
}

// Logical operations =============================================================================

Vector4 Vector4::operator& (const Vector4& b) const
{
    return Vector4(*this) &= b;
}

Vector4 Vector4::operator| (const Vector4& b) const
{
    return Vector4(*this) |= b;
}

Vector4 Vector4::operator^ (const Vector4& b) const
{
    return Vector4(*this) ^= b;
}

Vector4& Vector4::operator&= (const Vector4& b)
{
    i[0] &= b.i[0];
    i[1] &= b.i[1];
    i[2] &= b.i[2];
    i[3] &= b.i[3];
    return *this;
}

Vector4& Vector4::operator|= (const Vector4& b)
{
    i[0] |= b.i[0];
    i[1] |= b.i[1];
    i[2] |= b.i[2];
    i[3] |= b.i[3];
    return *this;
}

Vector4& Vector4::operator^= (const Vector4& b)
{
    i[0] ^= b.i[0];
    i[1] ^= b.i[1];
    i[2] ^= b.i[2];
    i[3] ^= b.i[3];
    return *this;
}

// Simple arithmetics =============================================================================

Vector4 Vector4::operator- () const
{
    return Vector4(-f[0], -f[1], -f[2], -f[3]);
}

Vector4 Vector4::operator+ (const Vector4& b) const
{
    return Vector4(*this) += b;
}

Vector4 Vector4::operator- (const Vector4& b) const
{
    return Vector4(*this) -= b;
}

Vector4 Vector4::operator* (const Vector4& b) const
{
    return Vector4(*this) *= b;
}

Vector4 Vector4::operator/ (const Vector4& b) const
{
    return Vector4(*this) /= b;
}

Vector4 Vector4::operator* (float b) const
{
    return Vector4(*this) *= b;
}

Vector4 Vector4::operator/ (float b) const
{
    return Vector4(*this) /= b;
}

Vector4 operator*(float a, const Vector4& b)
{
    return Vector4(a * b.f[0], a * b.f[1], a * b.f[2], a * b.f[3]);
}


Vector4& Vector4::operator+= (const Vector4& b)
{
    f[0] += b.f[0];
    f[1] += b.f[1];
    f[2] += b.f[2];
    f[3] += b.f[3];
    return *this;
}

Vector4& Vector4::operator-= (const Vector4& b)
{
    f[0] -= b.f[0];
    f[1] -= b.f[1];
    f[2] -= b.f[2];
    f[3] -= b.f[3];
    return *this;
}

Vector4& Vector4::operator*= (const Vector4& b)
{
    f[0] *= b.f[0];
    f[1] *= b.f[1];
    f[2] *= b.f[2];
    f[3] *= b.f[3];
    return *this;
}

Vector4& Vector4::operator/= (const Vector4& b)
{
    f[0] /= b.f[0];
    f[1] /= b.f[1];
    f[2] /= b.f[2];
    f[3] /= b.f[3];
    return *this;
}

Vector4& Vector4::operator*= (float b)
{
    f[0] *= b;
    f[1] *= b;
    f[2] *= b;
    f[3] *= b;
    return *this;
}

Vector4& Vector4::operator/= (float b)
{
    f[0] /= b;
    f[1] /= b;
    f[2] /= b;
    f[3] /= b;
    return *this;
}


Vector4 Vector4::MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
    return a * b + c;
}

Vector4 Vector4::MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
    return a * b - c;
}

Vector4 Vector4::NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c)
{
    return -(a * b) + c;
}

Vector4 Vector4::NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c)
{
    return -(a * b) - c;
}

Vector4 Vector4::Floor(const Vector4& v)
{
    return Vector4(floorf(v.f[0]), floorf(v.f[1]), floorf(v.f[2]), floorf(v.f[3]));
}

Vector4 Vector4::Sqrt(const Vector4& v)
{
    return Vector4(sqrtf(v.f[0]), v.f[1], v.f[2], v.f[3]);
}

Vector4 Vector4::Sqrt4(const Vector4& v)
{
    return Vector4(sqrtf(v.f[0]), sqrtf(v.f[1]), sqrtf(v.f[2]), sqrtf(v.f[3]));
}

Vector4 Vector4::Reciprocal(const Vector4& v)
{
    // this checks are required to avoid "potential divide by 0" warning
    return Vector4(v.f[0] != 0.0f ? 1.0f / v.f[0] : INFINITY,
                  v.f[1] != 0.0f ? 1.0f / v.f[1] : INFINITY,
                  v.f[2] != 0.0f ? 1.0f / v.f[2] : INFINITY,
                  v.f[3] != 0.0f ? 1.0f / v.f[3] : INFINITY);
}

Vector4 Vector4::Min(const Vector4& a, const Vector4& b)
{
    Vector4 vec;
    vec.f[0] = Math::Min<float>(a.f[0], b.f[0]);
    vec.f[1] = Math::Min<float>(a.f[1], b.f[1]);
    vec.f[2] = Math::Min<float>(a.f[2], b.f[2]);
    vec.f[3] = Math::Min<float>(a.f[3], b.f[3]);
    return vec;
}

Vector4 Vector4::Max(const Vector4& a, const Vector4& b)
{
    Vector4 vec;
    vec.f[0] = Math::Max<float>(a.f[0], b.f[0]);
    vec.f[1] = Math::Max<float>(a.f[1], b.f[1]);
    vec.f[2] = Math::Max<float>(a.f[2], b.f[2]);
    vec.f[3] = Math::Max<float>(a.f[3], b.f[3]);
    return vec;
}

Vector4 Vector4::Abs(const Vector4& v)
{
    return Vector4(fabsf(v.f[0]), fabsf(v.f[1]), fabsf(v.f[2]), fabsf(v.f[3]));
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight)
{
    Vector4 vec;
    vec.f[0] = v1.f[0] + weight.f[0] * (v2.f[0] - v1.f[0]);
    vec.f[1] = v1.f[1] + weight.f[1] * (v2.f[1] - v1.f[1]);
    vec.f[2] = v1.f[2] + weight.f[2] * (v2.f[2] - v1.f[2]);
    vec.f[3] = v1.f[3] + weight.f[3] * (v2.f[3] - v1.f[3]);
    return vec;
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, float weight)
{
    Vector4 vec;
    vec.f[0] = v1.f[0] + weight * (v2.f[0] - v1.f[0]);
    vec.f[1] = v1.f[1] + weight * (v2.f[1] - v1.f[1]);
    vec.f[2] = v1.f[2] + weight * (v2.f[2] - v1.f[2]);
    vec.f[3] = v1.f[3] + weight * (v2.f[3] - v1.f[3]);
    return vec;
}

// Comparison functions ===========================================================================

int Vector4::EqualMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.f[0] == v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] == v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] == v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] == v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector4::LessMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.f[0] < v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] < v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] < v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] < v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector4::LessEqMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.f[0] <= v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] <= v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] <= v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] <= v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector4::GreaterMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.f[0] > v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] > v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] > v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] > v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector4::GreaterEqMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.f[0] >= v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] >= v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] >= v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] >= v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector4::NotEqualMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.f[0] != v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] != v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] != v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] != v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

// 2D vector comparison functions =================================================================

bool Vector4::Equal2(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] == v2.f[0]) && (v1.f[1] == v2.f[1]);
}

bool Vector4::Less2(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] < v2.f[0]) && (v1.f[1] < v2.f[1]);
}

bool Vector4::LessEq2(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] <= v2.f[0]) && (v1.f[1] <= v2.f[1]);
}

bool Vector4::Greater2(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] > v2.f[0]) && (v1.f[1] > v2.f[1]);
}

bool Vector4::GreaterEq2(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] >= v2.f[0]) && (v1.f[1] >= v2.f[1]);
}

bool Vector4::NotEqual2(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] != v2.f[0]) && (v1.f[1] != v2.f[1]);
}

// 3D vector comparison functions =================================================================

bool Vector4::Equal3(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] == v2.f[0]) && (v1.f[1] == v2.f[1]) && (v1.f[2] == v2.f[2]);
}

bool Vector4::Less3(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] < v2.f[0]) && (v1.f[1] < v2.f[1]) && (v1.f[2] < v2.f[2]);
}

bool Vector4::LessEq3(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] <= v2.f[0]) && (v1.f[1] <= v2.f[1]) && (v1.f[2] <= v2.f[2]);
}

bool Vector4::Greater3(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] > v2.f[0]) && (v1.f[1] > v2.f[1]) && (v1.f[2] > v2.f[2]);
}

bool Vector4::GreaterEq3(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] >= v2.f[0]) && (v1.f[1] >= v2.f[1]) && (v1.f[2] >= v2.f[2]);
}

bool Vector4::NotEqual3(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] != v2.f[0]) && (v1.f[1] != v2.f[1]) && (v1.f[2] != v2.f[2]);
}

// 4D vector comparison functions =================================================================

bool Vector4::operator== (const Vector4& b) const
{
    return ((f[0] == b.f[0]) && (f[1] == b.f[1])) &&
           ((f[2] == b.f[2]) && (f[3] == b.f[3]));
}

bool Vector4::operator< (const Vector4& b) const
{
    return ((f[0] < b.f[0]) && (f[1] < b.f[1])) &&
           ((f[2] < b.f[2]) && (f[3] < b.f[3]));
}

bool Vector4::operator<= (const Vector4& b) const
{
    return ((f[0] <= b.f[0]) && (f[1] <= b.f[1])) &&
           ((f[2] <= b.f[2]) && (f[3] <= b.f[3]));
}

bool Vector4::operator> (const Vector4& b) const
{
    return ((f[0] > b.f[0]) && (f[1] > b.f[1])) &&
           ((f[2] > b.f[2]) && (f[3] > b.f[3]));
}

bool Vector4::operator>= (const Vector4& b) const
{
    return ((f[0] >= b.f[0]) && (f[1] >= b.f[1])) &&
           ((f[2] >= b.f[2]) && (f[3] >= b.f[3]));
}

bool Vector4::operator!= (const Vector4& b) const
{
    return ((f[0] != b.f[0]) && (f[1] != b.f[1])) &&
           ((f[2] != b.f[2]) && (f[3] != b.f[3]));
}

// Geometry functions =============================================================================

float Vector4::Dot2(const Vector4& v1, const Vector4& v2)
{
    return v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1];
}

Vector4 Vector4::Dot2V(const Vector4& v1, const Vector4& v2)
{
    return Vector4::Splat(Vector4::Dot2(v1, v2));
}

float Vector4::Dot3(const Vector4& v1, const Vector4& v2)
{
    return v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1] + v1.f[2] * v2.f[2];
}

Vector4 Vector4::Dot3V(const Vector4& v1, const Vector4& v2)
{
    return Vector4::Splat(Vector4::Dot3(v1, v2));
}

float Vector4::Dot4(const Vector4& v1, const Vector4& v2)
{
    return (v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1]) + (v1.f[2] * v2.f[2] + v1.f[3] * v2.f[3]);
}

Vector4 Vector4::Dot4V(const Vector4& v1, const Vector4& v2)
{
    return Vector4::Splat(Vector4::Dot4(v1, v2));
}

Vector4 Vector4::Cross3(const Vector4& v1, const Vector4& v2)
{
    Vector4 vec;
    vec.f[0] = v1.f[1] * v2.f[2] - v1.f[2] * v2.f[1];
    vec.f[1] = v1.f[2] * v2.f[0] - v1.f[0] * v2.f[2];
    vec.f[2] = v1.f[0] * v2.f[1] - v1.f[1] * v2.f[0];
    return vec;
}

float Vector4::Length2() const
{
    return sqrtf(f[0] * f[0] + f[1] * f[1]);
}

Vector4 Vector4::Length2V() const
{
    return Vector4::Splat(Length2());
}

float Vector4::Length3() const
{
    return sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
}

Vector4 Vector4::Length3V() const
{
    return Vector4::Splat(Length3());
}

float Vector4::Length4() const
{
    return sqrtf((f[0] * f[0] + f[1] * f[1]) + (f[2] * f[2] + f[3] * f[3]));
}

Vector4 Vector4::Length4V() const
{
    return Splat(Length4());
}

Vector4 Vector4::Normalized2() const
{
    float lenInv = 1.0f / Length2();
    return Vector4(f[0] * lenInv, f[1] * lenInv, 0.0f, 0.0f);
}

Vector4& Vector4::Normalize2()
{
    float lenInv = 1.0f / Length2();
    f[0] *= lenInv;
    f[1] *= lenInv;
    f[2] = 0.0f;
    f[3] = 0.0f;
    return *this;
}

Vector4 Vector4::Normalized3() const
{
    float lenInv = 1.0f / Length3();
    return Vector4(f[0] * lenInv, f[1] * lenInv, f[2] * lenInv, 0.0f);
}

Vector4& Vector4::Normalize3()
{
    float lenInv = 1.0f / Length3();
    x *= lenInv;
    y *= lenInv;
    z *= lenInv;
    w = 0.0f;
    return *this;
}

Vector4 Vector4::Normalized4() const
{
    float lenInv = 1.0f / Length4();
    return *this * lenInv;
}

Vector4& Vector4::Normalize4()
{
    float lenInv = 1.0f / Length4();
    *this *= lenInv;
    return *this;
}

Vector4 Vector4::Reflect3(const Vector4& i, const Vector4& n)
{
    float dot = Vector4::Dot3(i, n);
    return i - n * (dot + dot);
}

} // namespace Math
} // namespace NFE