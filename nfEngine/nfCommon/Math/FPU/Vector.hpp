/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU version of Vector class definitions.
 */

#pragma once

#if defined(NFE_USE_SSE)
#error "The header cannot be used when NFE_USE_SSE is defined!"
#endif

namespace NFE {
namespace Math {

// Constructors ===================================================================================

Vector::Vector()
{
    f[0] = f[1] = f[2] = f[3] = 0.0f;
}

Vector::Vector(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w)
{
}

Vector::Vector(int x, int y, int z, int w)
{
    i[0] = x;
    i[1] = y;
    i[2] = z;
    i[3] = w;
}

// copy array of 4 floats
Vector::Vector(const float* src)
{
    f[0] = src[0];
    f[1] = src[1];
    f[2] = src[2];
    f[3] = src[3];
}

Vector::Vector(const Float2& src)
{
    f[0] = src.x;
    f[1] = src.y;
    f[2] = 0.0f;
    f[3] = 0.0f;
}

Vector::Vector(const Float3& src)
{
    f[0] = src.x;
    f[1] = src.y;
    f[2] = src.z;
    f[3] = 0.0f;
}

Vector::Vector(const Float4& src)
{
    f[0] = src.x;
    f[1] = src.y;
    f[2] = src.z;
    f[3] = src.w;
}

void Vector::Set(float scalar)
{
    f[0] = scalar;
    f[1] = scalar;
    f[2] = scalar;
    f[3] = scalar;
}

// Load & store ===================================================================================

Vector Vector::Load4(const unsigned char* src)
{
    Vector vec;
    vec[0] = static_cast<float>(src[0]);
    vec[1] = static_cast<float>(src[1]);
    vec[2] = static_cast<float>(src[2]);
    vec[3] = static_cast<float>(src[3]);
    return vec;
}

void Vector::Store4(uint8* dest) const
{
    // TODO: saturate to <0, 255>
    dest[0] = static_cast<unsigned char>(f[0]);
    dest[1] = static_cast<unsigned char>(f[1]);
    dest[2] = static_cast<unsigned char>(f[2]);
    dest[3] = static_cast<unsigned char>(f[3]);
}

void Vector::Store(float* dest) const
{
    dest[0] = f[0];
    dest[1] = f[1];
    dest[2] = f[2];
    dest[3] = f[3];
}

void Vector::Store(Float2* dest) const
{
    dest->x = f[0];
    dest->y = f[1];
}

void Vector::Store(Float3* dest) const
{
    dest->x = f[0];
    dest->y = f[1];
    dest->z = f[2];
}

void Vector::Store(Float4* dest) const
{
    dest->x = f[0];
    dest->y = f[1];
    dest->z = f[2];
    dest->w = f[3];
}

template<bool negX, bool negY, bool negZ, bool negW>
Vector Vector::ChangeSign() const
{
    return Vector(
        negX ? -f[0] : f[0],
        negY ? -f[1] : f[1],
        negZ ? -f[2] : f[2],
        negW ? -f[3] : f[3]
    );
}

// Elements rearrangement =========================================================================

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Vector Vector::Swizzle() const
{
    static_assert(ix < 4, "Invalid X element index");
    static_assert(iy < 4, "Invalid Y element index");
    static_assert(iz < 4, "Invalid Z element index");
    static_assert(iw < 4, "Invalid W element index");

    return Vector(f[ix], f[iy], f[iz], f[iw]);
}

template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
Vector Vector::Blend(const Vector& a, const Vector& b)
{
    static_assert(ix < 2, "Invalid index for X component");
    static_assert(iy < 2, "Invalid index for Y component");
    static_assert(iz < 2, "Invalid index for Z component");
    static_assert(iw < 2, "Invalid index for W component");

    return Vector(ix == 0 ? a[0] : b[0],
                  iy == 0 ? a[1] : b[1],
                  iz == 0 ? a[2] : b[2],
                  iw == 0 ? a[3] : b[3]);
}

Vector Vector::SplatX() const
{
    return Vector(f[0], f[0], f[0], f[0]);
}

Vector Vector::SplatY() const
{
    return Vector(f[1], f[1], f[1], f[1]);
}

Vector Vector::SplatZ() const
{
    return Vector(f[2], f[2], f[2], f[2]);
}

Vector Vector::SplatW() const
{
    return Vector(f[3], f[3], f[3], f[3]);
}

Vector Vector::Splat(float f)
{
    return Vector(f, f, f, f);
}

Vector Vector::SelectBySign(const Vector& a, const Vector& b, const Vector& sel)
{
    Vector ret;
    ret[0] = sel[0] > 0.0f ? a[0] : b[0];
    ret[1] = sel[1] > 0.0f ? a[1] : b[1];
    ret[2] = sel[2] > 0.0f ? a[2] : b[2];
    ret[3] = sel[3] > 0.0f ? a[3] : b[3];
    return ret;
}

// Logical operations =============================================================================

Vector Vector::operator& (const Vector& b) const
{
    return Vector(*this) &= b;
}

Vector Vector::operator| (const Vector& b) const
{
    return Vector(*this) |= b;
}

Vector Vector::operator^ (const Vector& b) const
{
    return Vector(*this) ^= b;
}

Vector& Vector::operator&= (const Vector& b)
{
    i[0] &= b.i[0];
    i[1] &= b.i[1];
    i[2] &= b.i[2];
    i[3] &= b.i[3];
    return *this;
}

Vector& Vector::operator|= (const Vector& b)
{
    i[0] |= b.i[0];
    i[1] |= b.i[1];
    i[2] |= b.i[2];
    i[3] |= b.i[3];
    return *this;
}

Vector& Vector::operator^= (const Vector& b)
{
    i[0] ^= b.i[0];
    i[1] ^= b.i[1];
    i[2] ^= b.i[2];
    i[3] ^= b.i[3];
    return *this;
}

// Simple arithmetics =============================================================================

Vector Vector::operator- () const
{
    return Vector(-f[0], -f[1], -f[2], -f[3]);
}

Vector Vector::operator+ (const Vector& b) const
{
    return Vector(*this) += b;
}

Vector Vector::operator- (const Vector& b) const
{
    return Vector(*this) -= b;
}

Vector Vector::operator* (const Vector& b) const
{
    return Vector(*this) *= b;
}

Vector Vector::operator/ (const Vector& b) const
{
    return Vector(*this) /= b;
}

Vector Vector::operator* (float b) const
{
    return Vector(*this) *= b;
}

Vector Vector::operator/ (float b) const
{
    return Vector(*this) /= b;
}

Vector operator*(float a, const Vector& b)
{
    return Vector(a * b.f[0], a * b.f[1], a * b.f[2], a * b.f[3]);
}


Vector& Vector::operator+= (const Vector& b)
{
    f[0] += b.f[0];
    f[1] += b.f[1];
    f[2] += b.f[2];
    f[3] += b.f[3];
    return *this;
}

Vector& Vector::operator-= (const Vector& b)
{
    f[0] -= b.f[0];
    f[1] -= b.f[1];
    f[2] -= b.f[2];
    f[3] -= b.f[3];
    return *this;
}

Vector& Vector::operator*= (const Vector& b)
{
    f[0] *= b.f[0];
    f[1] *= b.f[1];
    f[2] *= b.f[2];
    f[3] *= b.f[3];
    return *this;
}

Vector& Vector::operator/= (const Vector& b)
{
    f[0] /= b.f[0];
    f[1] /= b.f[1];
    f[2] /= b.f[2];
    f[3] /= b.f[3];
    return *this;
}

Vector& Vector::operator*= (float b)
{
    f[0] *= b;
    f[1] *= b;
    f[2] *= b;
    f[3] *= b;
    return *this;
}

Vector& Vector::operator/= (float b)
{
    f[0] /= b;
    f[1] /= b;
    f[2] /= b;
    f[3] /= b;
    return *this;
}


Vector Vector::MulAndAdd(const Vector& a, const Vector& b, const Vector& c)
{
    return a * b + c;
}

Vector Vector::MulAndSub(const Vector& a, const Vector& b, const Vector& c)
{
    return a * b - c;
}

Vector Vector::NegMulAndAdd(const Vector& a, const Vector& b, const Vector& c)
{
    return -(a * b) + c;
}

Vector Vector::NegMulAndSub(const Vector& a, const Vector& b, const Vector& c)
{
    return -(a * b) - c;
}

Vector Vector::Floor(const Vector& v)
{
    return Vector(floorf(v.f[0]), floorf(v.f[1]), floorf(v.f[2]), floorf(v.f[3]));
}

Vector Vector::Sqrt(const Vector& v)
{
    return Vector(sqrtf(v.f[0]), v.f[1], v.f[2], v.f[3]);
}

Vector Vector::Sqrt4(const Vector& v)
{
    return Vector(sqrtf(v.f[0]), sqrtf(v.f[1]), sqrtf(v.f[2]), sqrtf(v.f[3]));
}

Vector Vector::Reciprocal(const Vector& v)
{
    // this checks are required to avoid "potential divide by 0" warning
    return Vector(v.f[0] != 0.0f ? 1.0f / v.f[0] : INFINITY,
                  v.f[1] != 0.0f ? 1.0f / v.f[1] : INFINITY,
                  v.f[2] != 0.0f ? 1.0f / v.f[2] : INFINITY,
                  v.f[3] != 0.0f ? 1.0f / v.f[3] : INFINITY);
}

Vector Vector::Min(const Vector& a, const Vector& b)
{
    Vector vec;
    vec.f[0] = Math::Min<float>(a.f[0], b.f[0]);
    vec.f[1] = Math::Min<float>(a.f[1], b.f[1]);
    vec.f[2] = Math::Min<float>(a.f[2], b.f[2]);
    vec.f[3] = Math::Min<float>(a.f[3], b.f[3]);
    return vec;
}

Vector Vector::Max(const Vector& a, const Vector& b)
{
    Vector vec;
    vec.f[0] = Math::Max<float>(a.f[0], b.f[0]);
    vec.f[1] = Math::Max<float>(a.f[1], b.f[1]);
    vec.f[2] = Math::Max<float>(a.f[2], b.f[2]);
    vec.f[3] = Math::Max<float>(a.f[3], b.f[3]);
    return vec;
}

Vector Vector::Abs(const Vector& v)
{
    return Vector(fabsf(v.f[0]), fabsf(v.f[1]), fabsf(v.f[2]), fabsf(v.f[3]));
}

Vector Vector::Lerp(const Vector& v1, const Vector& v2, const Vector& weight)
{
    Vector vec;
    vec.f[0] = v1.f[0] + weight.f[0] * (v2.f[0] - v1.f[0]);
    vec.f[1] = v1.f[1] + weight.f[1] * (v2.f[1] - v1.f[1]);
    vec.f[2] = v1.f[2] + weight.f[2] * (v2.f[2] - v1.f[2]);
    vec.f[3] = v1.f[3] + weight.f[3] * (v2.f[3] - v1.f[3]);
    return vec;
}

Vector Vector::Lerp(const Vector& v1, const Vector& v2, float weight)
{
    Vector vec;
    vec.f[0] = v1.f[0] + weight * (v2.f[0] - v1.f[0]);
    vec.f[1] = v1.f[1] + weight * (v2.f[1] - v1.f[1]);
    vec.f[2] = v1.f[2] + weight * (v2.f[2] - v1.f[2]);
    vec.f[3] = v1.f[3] + weight * (v2.f[3] - v1.f[3]);
    return vec;
}

// Comparison functions ===========================================================================

int Vector::EqualMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] == v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] == v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] == v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] == v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector::LessMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] < v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] < v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] < v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] < v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector::LessEqMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] <= v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] <= v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] <= v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] <= v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector::GreaterMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] > v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] > v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] > v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] > v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector::GreaterEqMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] >= v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] >= v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] >= v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] >= v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int Vector::NotEqualMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] != v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] != v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] != v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] != v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

// 2D vector comparison functions =================================================================

bool Vector::Equal2(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] == v2.f[0]) && (v1.f[1] == v2.f[1]);
}

bool Vector::Less2(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] < v2.f[0]) && (v1.f[1] < v2.f[1]);
}

bool Vector::LessEq2(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] <= v2.f[0]) && (v1.f[1] <= v2.f[1]);
}

bool Vector::Greater2(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] > v2.f[0]) && (v1.f[1] > v2.f[1]);
}

bool Vector::GreaterEq2(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] >= v2.f[0]) && (v1.f[1] >= v2.f[1]);
}

bool Vector::NotEqual2(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] != v2.f[0]) && (v1.f[1] != v2.f[1]);
}

// 3D vector comparison functions =================================================================

bool Vector::Equal3(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] == v2.f[0]) && (v1.f[1] == v2.f[1]) && (v1.f[2] == v2.f[2]);
}

bool Vector::Less3(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] < v2.f[0]) && (v1.f[1] < v2.f[1]) && (v1.f[2] < v2.f[2]);
}

bool Vector::LessEq3(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] <= v2.f[0]) && (v1.f[1] <= v2.f[1]) && (v1.f[2] <= v2.f[2]);
}

bool Vector::Greater3(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] > v2.f[0]) && (v1.f[1] > v2.f[1]) && (v1.f[2] > v2.f[2]);
}

bool Vector::GreaterEq3(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] >= v2.f[0]) && (v1.f[1] >= v2.f[1]) && (v1.f[2] >= v2.f[2]);
}

bool Vector::NotEqual3(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] != v2.f[0]) && (v1.f[1] != v2.f[1]) && (v1.f[2] != v2.f[2]);
}

// 4D vector comparison functions =================================================================

bool Vector::operator== (const Vector& b) const
{
    return ((f[0] == b.f[0]) && (f[1] == b.f[1])) &&
           ((f[2] == b.f[2]) && (f[3] == b.f[3]));
}

bool Vector::operator< (const Vector& b) const
{
    return ((f[0] < b.f[0]) && (f[1] < b.f[1])) &&
           ((f[2] < b.f[2]) && (f[3] < b.f[3]));
}

bool Vector::operator<= (const Vector& b) const
{
    return ((f[0] <= b.f[0]) && (f[1] <= b.f[1])) &&
           ((f[2] <= b.f[2]) && (f[3] <= b.f[3]));
}

bool Vector::operator> (const Vector& b) const
{
    return ((f[0] > b.f[0]) && (f[1] > b.f[1])) &&
           ((f[2] > b.f[2]) && (f[3] > b.f[3]));
}

bool Vector::operator>= (const Vector& b) const
{
    return ((f[0] >= b.f[0]) && (f[1] >= b.f[1])) &&
           ((f[2] >= b.f[2]) && (f[3] >= b.f[3]));
}

bool Vector::operator!= (const Vector& b) const
{
    return ((f[0] != b.f[0]) && (f[1] != b.f[1])) &&
           ((f[2] != b.f[2]) && (f[3] != b.f[3]));
}

// Geometry functions =============================================================================

float Vector::Dot2(const Vector& v1, const Vector& v2)
{
    return v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1];
}

Vector Vector::Dot2V(const Vector& v1, const Vector& v2)
{
    return Vector::Splat(Vector::Dot2(v1, v2));
}

float Vector::Dot3(const Vector& v1, const Vector& v2)
{
    return v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1] + v1.f[2] * v2.f[2];
}

Vector Vector::Dot3V(const Vector& v1, const Vector& v2)
{
    return Vector::Splat(Vector::Dot3(v1, v2));
}

float Vector::Dot4(const Vector& v1, const Vector& v2)
{
    return (v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1]) + (v1.f[2] * v2.f[2] + v1.f[3] * v2.f[3]);
}

Vector Vector::Dot4V(const Vector& v1, const Vector& v2)
{
    return Vector::Splat(Vector::Dot4(v1, v2));
}

Vector Vector::Cross3(const Vector& v1, const Vector& v2)
{
    Vector vec;
    vec.f[0] = v1.f[1] * v2.f[2] - v1.f[2] * v2.f[1];
    vec.f[1] = v1.f[2] * v2.f[0] - v1.f[0] * v2.f[2];
    vec.f[2] = v1.f[0] * v2.f[1] - v1.f[1] * v2.f[0];
    return vec;
}

float Vector::Length2() const
{
    return sqrtf(f[0] * f[0] + f[1] * f[1]);
}

Vector Vector::Length2V() const
{
    return Vector::Splat(Length2());
}

float Vector::Length3() const
{
    return sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
}

Vector Vector::Length3V() const
{
    return Vector::Splat(Length3());
}

float Vector::Length4() const
{
    return sqrtf((f[0] * f[0] + f[1] * f[1]) + (f[2] * f[2] + f[3] * f[3]));
}

Vector Vector::Length4V() const
{
    return Splat(Length4());
}

Vector Vector::Normalized2() const
{
    float lenInv = 1.0f / Length2();
    return Vector(f[0] * lenInv, f[1] * lenInv, 0.0f, 0.0f);
}

Vector& Vector::Normalize2()
{
    float lenInv = 1.0f / Length2();
    f[0] *= lenInv;
    f[1] *= lenInv;
    f[2] = 0.0f;
    f[3] = 0.0f;
    return *this;
}

Vector Vector::Normalized3() const
{
    float lenInv = 1.0f / Length3();
    return Vector(f[0] * lenInv, f[1] * lenInv, f[2] * lenInv, 0.0f);
}

Vector& Vector::Normalize3()
{
    float lenInv = 1.0f / Length3();
    x *= lenInv;
    y *= lenInv;
    z *= lenInv;
    w = 0.0f;
    return *this;
}

Vector Vector::Normalized4() const
{
    float lenInv = 1.0f / Length4();
    return *this * lenInv;
}

Vector& Vector::Normalize4()
{
    float lenInv = 1.0f / Length4();
    *this *= lenInv;
    return *this;
}

Vector Vector::Reflect3(const Vector& i, const Vector& n)
{
    float dot = Vector::Dot3(i, n);
    return i - n * (dot + dot);
}

} // namespace Math
} // namespace NFE