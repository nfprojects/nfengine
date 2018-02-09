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
    x = y = z = w = 0.0f;
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
    this->x = src[0];
    this->y = src[1];
    this->z = src[2];
    this->w = src[3];
}

Vector4::Vector4(const Float2& src)
{
    x = src.x;
    y = src.y;
    z = 0.0f;
    w = 0.0f;
}

Vector4::Vector4(const Float3& src)
{
    x = src.x;
    y = src.y;
    z = src.z;
    w = 0.0f;
}

Vector4::Vector4(const Float4& src)
{
    x = src.x;
    y = src.y;
    z = src.z;
    w = src.w;
}

void Vector4::Set(float scalar)
{
    x = scalar;
    y = scalar;
    z = scalar;
    w = scalar;
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
    dest[0] = static_cast<unsigned char>(x);
    dest[1] = static_cast<unsigned char>(y);
    dest[2] = static_cast<unsigned char>(z);
    dest[3] = static_cast<unsigned char>(w);
}

void Vector4::Store(float* dest) const
{
    dest[0] = x;
    dest[1] = y;
    dest[2] = z;
    dest[3] = w;
}

void Vector4::Store(Float2* dest) const
{
    dest->x = x;
    dest->y = y;
}

void Vector4::Store(Float3* dest) const
{
    dest->x = x;
    dest->y = y;
    dest->z = z;
}

void Vector4::Store(Float4* dest) const
{
    dest->x = x;
    dest->y = y;
    dest->z = z;
    dest->w = w;
}

template<bool negX, bool negY, bool negZ, bool negW>
Vector4 Vector4::ChangeSign() const
{
    return Vector4(
        negX ? -x : x,
        negY ? -y : y,
        negZ ? -z : z,
        negW ? -w : w
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
    return Vector4(x, x, x, x);
}

Vector4 Vector4::SplatY() const
{
    return Vector4(y, y, y, y);
}

Vector4 Vector4::SplatZ() const
{
    return Vector4(z, z, z, z);
}

Vector4 Vector4::SplatW() const
{
    return Vector4(w, w, w, w);
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
    return Vector4(-x, -y, -z, -w);
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
    return Vector4(a * b.x, a * b.y, a * b.z, a * b.w);
}


Vector4& Vector4::operator+= (const Vector4& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

Vector4& Vector4::operator-= (const Vector4& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

Vector4& Vector4::operator*= (const Vector4& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

Vector4& Vector4::operator/= (const Vector4& b)
{
    x /= b.x;
    y /= b.y;
    z /= b.z;
    w /= b.w;
    return *this;
}

Vector4& Vector4::operator*= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

Vector4& Vector4::operator/= (float b)
{
    x /= b;
    y /= b;
    z /= b;
    w /= b;
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
    return Vector4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

Vector4 Vector4::Sqrt(const Vector4& v)
{
    return Vector4(sqrtf(v.x), v.y, v.z, v.w);
}

Vector4 Vector4::Sqrt4(const Vector4& v)
{
    return Vector4(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w));
}

Vector4 Vector4::Reciprocal(const Vector4& v)
{
    // this checks are required to avoid "potential divide by 0" warning
    return Vector4(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY,
                  v.z != 0.0f ? 1.0f / v.z : INFINITY,
                  v.w != 0.0f ? 1.0f / v.w : INFINITY);
}

Vector4 Vector4::Min(const Vector4& a, const Vector4& b)
{
    Vector4 vec;
    vec.x = Math::Min<float>(a.x, b.x);
    vec.y = Math::Min<float>(a.y, b.y);
    vec.z = Math::Min<float>(a.z, b.z);
    vec.w = Math::Min<float>(a.w, b.w);
    return vec;
}

Vector4 Vector4::Max(const Vector4& a, const Vector4& b)
{
    Vector4 vec;
    vec.x = Math::Max<float>(a.x, b.x);
    vec.y = Math::Max<float>(a.y, b.y);
    vec.z = Math::Max<float>(a.z, b.z);
    vec.w = Math::Max<float>(a.w, b.w);
    return vec;
}

Vector4 Vector4::Abs(const Vector4& v)
{
    return Vector4(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w));
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight)
{
    Vector4 vec;
    vec.x = v1.x + weight.x * (v2.x - v1.x);
    vec.y = v1.y + weight.y * (v2.y - v1.y);
    vec.z = v1.z + weight.z * (v2.z - v1.z);
    vec.w = v1.w + weight.w * (v2.w - v1.w);
    return vec;
}

Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, float weight)
{
    Vector4 vec;
    vec.x = v1.x + weight * (v2.x - v1.x);
    vec.y = v1.y + weight * (v2.y - v1.y);
    vec.z = v1.z + weight * (v2.z - v1.z);
    vec.w = v1.w + weight * (v2.w - v1.w);
    return vec;
}

// Comparison functions ===========================================================================

int Vector4::EqualMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.x == v2.x) ? (1 << 0) : 0;
    ret |= (v1.y == v2.y) ? (1 << 1) : 0;
    ret |= (v1.z == v2.z) ? (1 << 2) : 0;
    ret |= (v1.w == v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector4::LessMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.x < v2.x) ? (1 << 0) : 0;
    ret |= (v1.y < v2.y) ? (1 << 1) : 0;
    ret |= (v1.z < v2.z) ? (1 << 2) : 0;
    ret |= (v1.w < v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector4::LessEqMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.x <= v2.x) ? (1 << 0) : 0;
    ret |= (v1.y <= v2.y) ? (1 << 1) : 0;
    ret |= (v1.z <= v2.z) ? (1 << 2) : 0;
    ret |= (v1.w <= v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector4::GreaterMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.x > v2.x) ? (1 << 0) : 0;
    ret |= (v1.y > v2.y) ? (1 << 1) : 0;
    ret |= (v1.z > v2.z) ? (1 << 2) : 0;
    ret |= (v1.w > v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector4::GreaterEqMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.x >= v2.x) ? (1 << 0) : 0;
    ret |= (v1.y >= v2.y) ? (1 << 1) : 0;
    ret |= (v1.z >= v2.z) ? (1 << 2) : 0;
    ret |= (v1.w >= v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector4::NotEqualMask(const Vector4& v1, const Vector4& v2)
{
    int ret = 0;
    ret |= (v1.x != v2.x) ? (1 << 0) : 0;
    ret |= (v1.y != v2.y) ? (1 << 1) : 0;
    ret |= (v1.z != v2.z) ? (1 << 2) : 0;
    ret |= (v1.w != v2.w) ? (1 << 3) : 0;
    return ret;
}

// 2D vector comparison functions =================================================================

bool Vector4::Equal2(const Vector4& v1, const Vector4& v2)
{
    return (v1.x == v2.x) && (v1.y == v2.y);
}

bool Vector4::Less2(const Vector4& v1, const Vector4& v2)
{
    return (v1.x < v2.x) && (v1.y < v2.y);
}

bool Vector4::LessEq2(const Vector4& v1, const Vector4& v2)
{
    return (v1.x <= v2.x) && (v1.y <= v2.y);
}

bool Vector4::Greater2(const Vector4& v1, const Vector4& v2)
{
    return (v1.x > v2.x) && (v1.y > v2.y);
}

bool Vector4::GreaterEq2(const Vector4& v1, const Vector4& v2)
{
    return (v1.x >= v2.x) && (v1.y >= v2.y);
}

bool Vector4::NotEqual2(const Vector4& v1, const Vector4& v2)
{
    return (v1.x != v2.x) && (v1.y != v2.y);
}

// 3D vector comparison functions =================================================================

bool Vector4::Equal3(const Vector4& v1, const Vector4& v2)
{
    return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
}

bool Vector4::Less3(const Vector4& v1, const Vector4& v2)
{
    return (v1.x < v2.x) && (v1.y < v2.y) && (v1.z < v2.z);
}

bool Vector4::LessEq3(const Vector4& v1, const Vector4& v2)
{
    return (v1.x <= v2.x) && (v1.y <= v2.y) && (v1.z <= v2.z);
}

bool Vector4::Greater3(const Vector4& v1, const Vector4& v2)
{
    return (v1.x > v2.x) && (v1.y > v2.y) && (v1.z > v2.z);
}

bool Vector4::GreaterEq3(const Vector4& v1, const Vector4& v2)
{
    return (v1.x >= v2.x) && (v1.y >= v2.y) && (v1.z >= v2.z);
}

bool Vector4::NotEqual3(const Vector4& v1, const Vector4& v2)
{
    return (v1.x != v2.x) && (v1.y != v2.y) && (v1.z != v2.z);
}

// 4D vector comparison functions =================================================================

bool Vector4::operator== (const Vector4& b) const
{
    return ((x == b.x) && (y == b.y)) &&
           ((z == b.z) && (w == b.w));
}

bool Vector4::operator< (const Vector4& b) const
{
    return ((x < b.x) && (y < b.y)) &&
           ((z < b.z) && (w < b.w));
}

bool Vector4::operator<= (const Vector4& b) const
{
    return ((x <= b.x) && (y <= b.y)) &&
           ((z <= b.z) && (w <= b.w));
}

bool Vector4::operator> (const Vector4& b) const
{
    return ((x > b.x) && (y > b.y)) &&
           ((z > b.z) && (w > b.w));
}

bool Vector4::operator>= (const Vector4& b) const
{
    return ((x >= b.x) && (y >= b.y)) &&
           ((z >= b.z) && (w >= b.w));
}

bool Vector4::operator!= (const Vector4& b) const
{
    return ((x != b.x) && (y != b.y)) &&
           ((z != b.z) && (w != b.w));
}

// Geometry functions =============================================================================

float Vector4::Dot2(const Vector4& v1, const Vector4& v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

Vector4 Vector4::Dot2V(const Vector4& v1, const Vector4& v2)
{
    return Vector4::Splat(Vector4::Dot2(v1, v2));
}

float Vector4::Dot3(const Vector4& v1, const Vector4& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector4 Vector4::Dot3V(const Vector4& v1, const Vector4& v2)
{
    return Vector4::Splat(Vector4::Dot3(v1, v2));
}

float Vector4::Dot4(const Vector4& v1, const Vector4& v2)
{
    return (v1.x * v2.x + v1.y * v2.y) + (v1.z * v2.z + v1.w * v2.w);
}

Vector4 Vector4::Dot4V(const Vector4& v1, const Vector4& v2)
{
    return Vector4::Splat(Vector4::Dot4(v1, v2));
}

Vector4 Vector4::Cross3(const Vector4& v1, const Vector4& v2)
{
    Vector4 vec;
    vec.x = v1.y * v2.z - v1.z * v2.y;
    vec.y = v1.z * v2.x - v1.x * v2.z;
    vec.z = v1.x * v2.y - v1.y * v2.x;
    return vec;
}

float Vector4::Length2() const
{
    return sqrtf(x * x + y * y);
}

Vector4 Vector4::Length2V() const
{
    return Vector4::Splat(Length2());
}

float Vector4::Length3() const
{
    return sqrtf(x * x + y * y + z * z);
}

Vector4 Vector4::Length3V() const
{
    return Vector4::Splat(Length3());
}

float Vector4::Length4() const
{
    return sqrtf((x * x + y * y) + (z * z + w * w));
}

Vector4 Vector4::Length4V() const
{
    return Splat(Length4());
}

Vector4 Vector4::Normalized2() const
{
    float lenInv = 1.0f / Length2();
    return Vector4(x * lenInv, y * lenInv, 0.0f, 0.0f);
}

Vector4& Vector4::Normalize2()
{
    float lenInv = 1.0f / Length2();
    x *= lenInv;
    y *= lenInv;
    z = 0.0f;
    w = 0.0f;
    return *this;
}

Vector4 Vector4::Normalized3() const
{
    float lenInv = 1.0f / Length3();
    return Vector4(x * lenInv, y * lenInv, z * lenInv, 0.0f);
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