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
    x = y = z = w = 0.0f;
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
    this->x = src[0];
    this->y = src[1];
    this->z = src[2];
    this->w = src[3];
}

Vector::Vector(const Float2& src)
{
    x = src.x;
    y = src.y;
    z = 0.0f;
    w = 0.0f;
}

Vector::Vector(const Float3& src)
{
    x = src.x;
    y = src.y;
    z = src.z;
    w = 0.0f;
}

Vector::Vector(const Float4& src)
{
    x = src.x;
    y = src.y;
    z = src.z;
    w = src.w;
}

void Vector::Set(float scalar)
{
    x = scalar;
    y = scalar;
    z = scalar;
    w = scalar;
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
    dest[0] = static_cast<unsigned char>(x);
    dest[1] = static_cast<unsigned char>(y);
    dest[2] = static_cast<unsigned char>(z);
    dest[3] = static_cast<unsigned char>(w);
}

void Vector::Store(float* dest) const
{
    dest[0] = x;
    dest[1] = y;
    dest[2] = z;
    dest[3] = w;
}

void Vector::Store(Float2* dest) const
{
    dest->x = x;
    dest->y = y;
}

void Vector::Store(Float3* dest) const
{
    dest->x = x;
    dest->y = y;
    dest->z = z;
}

void Vector::Store(Float4* dest) const
{
    dest->x = x;
    dest->y = y;
    dest->z = z;
    dest->w = w;
}

template<bool negX, bool negY, bool negZ, bool negW>
Vector Vector::ChangeSign() const
{
    return Vector(
        negX ? -x : x,
        negY ? -y : y,
        negZ ? -z : z,
        negW ? -w : w
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
    return Vector(x, x, x, x);
}

Vector Vector::SplatY() const
{
    return Vector(y, y, y, y);
}

Vector Vector::SplatZ() const
{
    return Vector(z, z, z, z);
}

Vector Vector::SplatW() const
{
    return Vector(w, w, w, w);
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
    return Vector(-x, -y, -z, -w);
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
    return Vector(a * b.x, a * b.y, a * b.z, a * b.w);
}


Vector& Vector::operator+= (const Vector& b)
{
    x += b.x;
    y += b.y;
    z += b.z;
    w += b.w;
    return *this;
}

Vector& Vector::operator-= (const Vector& b)
{
    x -= b.x;
    y -= b.y;
    z -= b.z;
    w -= b.w;
    return *this;
}

Vector& Vector::operator*= (const Vector& b)
{
    x *= b.x;
    y *= b.y;
    z *= b.z;
    w *= b.w;
    return *this;
}

Vector& Vector::operator/= (const Vector& b)
{
    x /= b.x;
    y /= b.y;
    z /= b.z;
    w /= b.w;
    return *this;
}

Vector& Vector::operator*= (float b)
{
    x *= b;
    y *= b;
    z *= b;
    w *= b;
    return *this;
}

Vector& Vector::operator/= (float b)
{
    x /= b;
    y /= b;
    z /= b;
    w /= b;
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
    return Vector(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

Vector Vector::Sqrt(const Vector& v)
{
    return Vector(sqrtf(v.x), v.y, v.z, v.w);
}

Vector Vector::Sqrt4(const Vector& v)
{
    return Vector(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w));
}

Vector Vector::Reciprocal(const Vector& v)
{
    // this checks are required to avoid "potential divide by 0" warning
    return Vector(v.x != 0.0f ? 1.0f / v.x : INFINITY,
                  v.y != 0.0f ? 1.0f / v.y : INFINITY,
                  v.z != 0.0f ? 1.0f / v.z : INFINITY,
                  v.w != 0.0f ? 1.0f / v.w : INFINITY);
}

Vector Vector::Min(const Vector& a, const Vector& b)
{
    Vector vec;
    vec.x = Math::Min<float>(a.x, b.x);
    vec.y = Math::Min<float>(a.y, b.y);
    vec.z = Math::Min<float>(a.z, b.z);
    vec.w = Math::Min<float>(a.w, b.w);
    return vec;
}

Vector Vector::Max(const Vector& a, const Vector& b)
{
    Vector vec;
    vec.x = Math::Max<float>(a.x, b.x);
    vec.y = Math::Max<float>(a.y, b.y);
    vec.z = Math::Max<float>(a.z, b.z);
    vec.w = Math::Max<float>(a.w, b.w);
    return vec;
}

Vector Vector::Abs(const Vector& v)
{
    return Vector(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w));
}

Vector Vector::Lerp(const Vector& v1, const Vector& v2, const Vector& weight)
{
    Vector vec;
    vec.x = v1.x + weight.x * (v2.x - v1.x);
    vec.y = v1.y + weight.y * (v2.y - v1.y);
    vec.z = v1.z + weight.z * (v2.z - v1.z);
    vec.w = v1.w + weight.w * (v2.w - v1.w);
    return vec;
}

Vector Vector::Lerp(const Vector& v1, const Vector& v2, float weight)
{
    Vector vec;
    vec.x = v1.x + weight * (v2.x - v1.x);
    vec.y = v1.y + weight * (v2.y - v1.y);
    vec.z = v1.z + weight * (v2.z - v1.z);
    vec.w = v1.w + weight * (v2.w - v1.w);
    return vec;
}

// Comparison functions ===========================================================================

int Vector::EqualMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.x == v2.x) ? (1 << 0) : 0;
    ret |= (v1.y == v2.y) ? (1 << 1) : 0;
    ret |= (v1.z == v2.z) ? (1 << 2) : 0;
    ret |= (v1.w == v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector::LessMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.x < v2.x) ? (1 << 0) : 0;
    ret |= (v1.y < v2.y) ? (1 << 1) : 0;
    ret |= (v1.z < v2.z) ? (1 << 2) : 0;
    ret |= (v1.w < v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector::LessEqMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.x <= v2.x) ? (1 << 0) : 0;
    ret |= (v1.y <= v2.y) ? (1 << 1) : 0;
    ret |= (v1.z <= v2.z) ? (1 << 2) : 0;
    ret |= (v1.w <= v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector::GreaterMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.x > v2.x) ? (1 << 0) : 0;
    ret |= (v1.y > v2.y) ? (1 << 1) : 0;
    ret |= (v1.z > v2.z) ? (1 << 2) : 0;
    ret |= (v1.w > v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector::GreaterEqMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.x >= v2.x) ? (1 << 0) : 0;
    ret |= (v1.y >= v2.y) ? (1 << 1) : 0;
    ret |= (v1.z >= v2.z) ? (1 << 2) : 0;
    ret |= (v1.w >= v2.w) ? (1 << 3) : 0;
    return ret;
}

int Vector::NotEqualMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.x != v2.x) ? (1 << 0) : 0;
    ret |= (v1.y != v2.y) ? (1 << 1) : 0;
    ret |= (v1.z != v2.z) ? (1 << 2) : 0;
    ret |= (v1.w != v2.w) ? (1 << 3) : 0;
    return ret;
}

// 2D vector comparison functions =================================================================

bool Vector::Equal2(const Vector& v1, const Vector& v2)
{
    return (v1.x == v2.x) && (v1.y == v2.y);
}

bool Vector::Less2(const Vector& v1, const Vector& v2)
{
    return (v1.x < v2.x) && (v1.y < v2.y);
}

bool Vector::LessEq2(const Vector& v1, const Vector& v2)
{
    return (v1.x <= v2.x) && (v1.y <= v2.y);
}

bool Vector::Greater2(const Vector& v1, const Vector& v2)
{
    return (v1.x > v2.x) && (v1.y > v2.y);
}

bool Vector::GreaterEq2(const Vector& v1, const Vector& v2)
{
    return (v1.x >= v2.x) && (v1.y >= v2.y);
}

bool Vector::NotEqual2(const Vector& v1, const Vector& v2)
{
    return (v1.x != v2.x) && (v1.y != v2.y);
}

// 3D vector comparison functions =================================================================

bool Vector::Equal3(const Vector& v1, const Vector& v2)
{
    return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
}

bool Vector::Less3(const Vector& v1, const Vector& v2)
{
    return (v1.x < v2.x) && (v1.y < v2.y) && (v1.z < v2.z);
}

bool Vector::LessEq3(const Vector& v1, const Vector& v2)
{
    return (v1.x <= v2.x) && (v1.y <= v2.y) && (v1.z <= v2.z);
}

bool Vector::Greater3(const Vector& v1, const Vector& v2)
{
    return (v1.x > v2.x) && (v1.y > v2.y) && (v1.z > v2.z);
}

bool Vector::GreaterEq3(const Vector& v1, const Vector& v2)
{
    return (v1.x >= v2.x) && (v1.y >= v2.y) && (v1.z >= v2.z);
}

bool Vector::NotEqual3(const Vector& v1, const Vector& v2)
{
    return (v1.x != v2.x) && (v1.y != v2.y) && (v1.z != v2.z);
}

// 4D vector comparison functions =================================================================

bool Vector::operator== (const Vector& b) const
{
    return ((x == b.x) && (y == b.y)) &&
           ((z == b.z) && (w == b.w));
}

bool Vector::operator< (const Vector& b) const
{
    return ((x < b.x) && (y < b.y)) &&
           ((z < b.z) && (w < b.w));
}

bool Vector::operator<= (const Vector& b) const
{
    return ((x <= b.x) && (y <= b.y)) &&
           ((z <= b.z) && (w <= b.w));
}

bool Vector::operator> (const Vector& b) const
{
    return ((x > b.x) && (y > b.y)) &&
           ((z > b.z) && (w > b.w));
}

bool Vector::operator>= (const Vector& b) const
{
    return ((x >= b.x) && (y >= b.y)) &&
           ((z >= b.z) && (w >= b.w));
}

bool Vector::operator!= (const Vector& b) const
{
    return ((x != b.x) && (y != b.y)) &&
           ((z != b.z) && (w != b.w));
}

// Geometry functions =============================================================================

float Vector::Dot2(const Vector& v1, const Vector& v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

Vector Vector::Dot2V(const Vector& v1, const Vector& v2)
{
    return Vector::Splat(Vector::Dot2(v1, v2));
}

float Vector::Dot3(const Vector& v1, const Vector& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector Vector::Dot3V(const Vector& v1, const Vector& v2)
{
    return Vector::Splat(Vector::Dot3(v1, v2));
}

float Vector::Dot4(const Vector& v1, const Vector& v2)
{
    return (v1.x * v2.x + v1.y * v2.y) + (v1.z * v2.z + v1.w * v2.w);
}

Vector Vector::Dot4V(const Vector& v1, const Vector& v2)
{
    return Vector::Splat(Vector::Dot4(v1, v2));
}

Vector Vector::Cross3(const Vector& v1, const Vector& v2)
{
    Vector vec;
    vec.x = v1.y * v2.z - v1.z * v2.y;
    vec.y = v1.z * v2.x - v1.x * v2.z;
    vec.z = v1.x * v2.y - v1.y * v2.x;
    return vec;
}

float Vector::Length2() const
{
    return sqrtf(x * x + y * y);
}

Vector Vector::Length2V() const
{
    return Vector::Splat(Length2());
}

float Vector::Length3() const
{
    return sqrtf(x * x + y * y + z * z);
}

Vector Vector::Length3V() const
{
    return Vector::Splat(Length3());
}

float Vector::Length4() const
{
    return sqrtf((x * x + y * y) + (z * z + w * w));
}

Vector Vector::Length4V() const
{
    return Splat(Length4());
}

Vector Vector::Normalized2() const
{
    float lenInv = 1.0f / Length2();
    return Vector(x * lenInv, y * lenInv, 0.0f, 0.0f);
}

Vector& Vector::Normalize2()
{
    float lenInv = 1.0f / Length2();
    x *= lenInv;
    y *= lenInv;
    z = 0.0f;
    w = 0.0f;
    return *this;
}

Vector Vector::Normalized3() const
{
    float lenInv = 1.0f / Length3();
    return Vector(x * lenInv, y * lenInv, z * lenInv, 0.0f);
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