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
{
    f[0] = x;
    f[1] = y;
    f[2] = z;
    f[3] = w;
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

Vector VectorLoadUChar4(const unsigned char* src)
{
    Vector vec;
    vec[0] = static_cast<float>(src[0]);
    vec[1] = static_cast<float>(src[1]);
    vec[2] = static_cast<float>(src[2]);
    vec[3] = static_cast<float>(src[3]);
    return vec;
}

void Vector::Store(uint8* dest) const
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

Vector VectorFloor(const Vector& v)
{
    return Vector(floorf(v.f[0]), floorf(v.f[1]), floorf(v.f[2]), floorf(v.f[3]));
}

Vector VectorSqrt(const Vector& v)
{
    return Vector(sqrtf(v.f[0]), v.f[1], v.f[2], v.f[3]);
}

Vector VectorSqrt4(const Vector& v)
{
    return Vector(sqrtf(v.f[0]), sqrtf(v.f[1]), sqrtf(v.f[2]), sqrtf(v.f[3]));
}

Vector VectorReciprocal(const Vector& v)
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
    vec.f[0] = Min(a.f[0], b.f[0]);
    vec.f[1] = Min(a.f[1], b.f[1]);
    vec.f[2] = Min(a.f[2], b.f[2]);
    vec.f[3] = Min(a.f[3], b.f[3]);
    return vec;
}

Vector Vector::Max(const Vector& a, const Vector& b)
{
    Vector vec;
    vec.f[0] = Max(a.f[0], b.f[0]);
    vec.f[1] = Max(a.f[1], b.f[1]);
    vec.f[2] = Max(a.f[2], b.f[2]);
    vec.f[3] = Max(a.f[3], b.f[3]);
    return vec;
}

Vector Vector::Abs(const Vector& v)
{
    return Vector(fabsf(v.f[0]), fabsf(v.f[1]), fabsf(v.f[2]), fabsf(v.f[3]));
}

Vector VectorLerp(const Vector& v1, const Vector& v2, const Vector& weight)
{
    Vector vec;
    vec.f[0] = v1.f[0] + weight.f[0] * (v2.f[0] - v1.f[0]);
    vec.f[1] = v1.f[1] + weight.f[1] * (v2.f[1] - v1.f[1]);
    vec.f[2] = v1.f[2] + weight.f[2] * (v2.f[2] - v1.f[2]);
    vec.f[3] = v1.f[3] + weight.f[3] * (v2.f[3] - v1.f[3]);
    return vec;
}

Vector VectorLerp(const Vector& v1, const Vector& v2, float weight)
{
    Vector vec;
    vec.f[0] = v1.f[0] + weight * (v2.f[0] - v1.f[0]);
    vec.f[1] = v1.f[1] + weight * (v2.f[1] - v1.f[1]);
    vec.f[2] = v1.f[2] + weight * (v2.f[2] - v1.f[2]);
    vec.f[3] = v1.f[3] + weight * (v2.f[3] - v1.f[3]);
    return vec;
}

// Comparison functions ===========================================================================

int VectorEqualMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] == v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] == v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] == v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] == v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int VectorLessMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] < v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] < v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] < v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] < v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int VectorLessEqMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] <= v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] <= v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] <= v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] <= v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int VectorGreaterMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] > v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] > v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] > v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] > v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int VectorGreaterEqMask(const Vector& v1, const Vector& v2)
{
    int ret = 0;
    ret |= (v1.f[0] >= v2.f[0]) ? (1 << 0) : 0;
    ret |= (v1.f[1] >= v2.f[1]) ? (1 << 1) : 0;
    ret |= (v1.f[2] >= v2.f[2]) ? (1 << 2) : 0;
    ret |= (v1.f[3] >= v2.f[3]) ? (1 << 3) : 0;
    return ret;
}

int VectorNotEqualMask(const Vector& v1, const Vector& v2)
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

float Vector::Dot3(const Vector& v1, const Vector& v2)
{
    return v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1] + v1.f[2] * v2.f[2];
}

Vector Vector::Dot3V(const Vector& v1, const Vector& v2)
{
    return Vector::Splat(Vector::Dot3(v1, v2));
}

Vector Vector::Dot4(const Vector& v1, const Vector& v2)
{
    return Vector::Splat((v1.f[0] * v2.f[0] + v1.f[1] * v2.f[1]) +
                       (v1.f[2] * v2.f[2] + v1.f[3] * v2.f[3]));
}

Vector Vector::Cross3(const Vector& v1, const Vector& v2)
{
    Vector vec;
    vec.f[0] = v1.f[1] * v2.f[2] - v1.f[2] * v2.f[1];
    vec.f[1] = v1.f[2] * v2.f[0] - v1.f[0] * v2.f[2];
    vec.f[2] = v1.f[0] * v2.f[1] - v1.f[1] * v2.f[0];
    return vec;
}

float Vector::Length3() const
{
    return sqrtf(v.f[0] * v.f[0] + v.f[1] * v.f[1] + v.f[2] * v.f[2]);
}

Vector Vector::Length3V() const
{
    return Vector::Splat(Length3(v));
}

float Vector::Length4() const
{
    return sqrtf((v.f[0] * v.f[0] + v.f[1] * v.f[1]) + (v.f[2] * v.f[2] + v.f[3] * v.f[3]));
}

Vector Vector::Length4V() const
{
    return Splat(Length4(v));
}

Vector Vector::Normalize3(const Vector& v)
{
    float lenInv = 1.0f / Vector::Length3(v);
    return Vector(v.f[0] * lenInv, v.f[1] * lenInv, v.f[2] * lenInv, 0.0f);
}

Vector Vector::Normalize4(const Vector& v)
{
    float lenInv = 1.0f / VectorLength4f(v);
    return v * lenInv;
}

Vector Vector::Reflect3(const Vector& i, const Vector& n)
{
    float dot = Vector::Dot3(i, n);
    return i - n * (dot + dot);
}

Vector Vector::PlaneFromPoints(const Vector& p1, const Vector& p2, const Vector& p3)
{
    Vector v21 = p1 - p2;
    Vector v31 = p1 - p3;
    Vector n = VectorNormalize3(Vector::Cross3(v21, v31));
    float d = Vector::Dot3(n, p1);
    return Vector(n.f[0], n.f[1], n.f[2], -d);
}

Vector Vector::PlaneFromNormalAndPoint(const Vector& normal, const Vector& p)
{
    float d = Vector::Dot3(normal, p);
    return Vector(normal.f[0], normal.f[1], normal.f[2], -d);
}

bool Vector::PlanePointSide(const Vector& plane, const Vector& point)
{
    float d = Vector::Dot3(plane, point) + plane.f[3];
    return d > 0.0f;
}

} // namespace Math
} // namespace NFE