#pragma once

#include "Math.hpp"

#include "Half.hpp"
#include "Vec2f.hpp"
#include "Vec3f.hpp"
#include "Vec4fU.hpp"

#include "../Memory/Aligned.hpp"

namespace NFE {
namespace Math {

/**
 * Boolean vector for Vec4f type.
 */
struct NFE_ALIGN(16) VecBool4f : public Common::Aligned<16>
{
    VecBool4f() = default;

    NFE_FORCE_INLINE explicit VecBool4f(bool x, bool y, bool z, bool w);

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE VecBool4f(const __m128 other) : v(other) { }
    NFE_FORCE_INLINE VecBool4f(const __m128i other) : v(_mm_castsi128_ps(other)) { }
    NFE_FORCE_INLINE operator __m128() const { return v; }
    NFE_FORCE_INLINE operator __m128i() const { return _mm_castps_si128(v); }
#endif // NFE_USE_SSE

    template<uint32 index>
    NFE_FORCE_INLINE bool Get() const;

    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_FORCE_INLINE const VecBool4f Swizzle() const;

    // combine into 4-bit mask
    NFE_FORCE_INLINE int32 GetMask() const;

    NFE_FORCE_INLINE bool All() const;
    NFE_FORCE_INLINE bool None() const;
    NFE_FORCE_INLINE bool Any() const;

    NFE_FORCE_INLINE bool All3() const;
    NFE_FORCE_INLINE bool None3() const;
    NFE_FORCE_INLINE bool Any3() const;

    NFE_FORCE_INLINE const VecBool4f operator & (const VecBool4f rhs) const;
    NFE_FORCE_INLINE const VecBool4f operator | (const VecBool4f rhs) const;
    NFE_FORCE_INLINE const VecBool4f operator ^ (const VecBool4f rhs) const;

    NFE_FORCE_INLINE bool operator == (const VecBool4f rhs) const;

private:
    friend struct Vec4f;
    friend struct VecBool4i;
    friend struct VecBool8f;

#ifdef NFE_USE_SSE
    __m128 v;
#else
    bool b[4];
#endif // NFE_USE_SSE
};

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 4-element SIMD vector.
 */
struct NFE_ALIGN(16) Vec4f : public Common::Aligned<16>
{
    NFE_DECLARE_CLASS(Vec4f)

public:

    using BoolType = VecBool4f;

    NFE_UNNAMED_STRUCT union
    {
        float f[4];
        int32 i[4];
        uint32 u[4];

#ifdef NFE_USE_SSE
        __m128 v;
        __m128i vi;
#endif // NFE_USE_SSE

        NFE_UNNAMED_STRUCT struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };

    NFE_FORCE_INLINE Vec4f();
    NFE_FORCE_INLINE Vec4f(const Vec4f& other);
    NFE_FORCE_INLINE static const Vec4f Zero();
    NFE_FORCE_INLINE explicit Vec4f(const float scalar); // splat
    NFE_FORCE_INLINE explicit Vec4f(const int32 scalar); // splat
    NFE_FORCE_INLINE explicit Vec4f(const uint32 scalar); // splat
    NFE_FORCE_INLINE Vec4f(const float x, const float y, const float z = 0.0f, const float w = 0.0f);
    NFE_FORCE_INLINE Vec4f(const int32 x, const int32 y, const int32 z = 0, const int32 w = 0);
    NFE_FORCE_INLINE Vec4f(const uint32 x, const uint32 y, const uint32 z = 0u, const uint32 w = 0u);
    NFE_FORCE_INLINE explicit Vec4f(const float* src);
    NFE_FORCE_INLINE explicit Vec4f(const Vec2f& src);
    NFE_FORCE_INLINE explicit Vec4f(const Vec3f& src);
    NFE_FORCE_INLINE explicit Vec4f(const Vec4fU& src);
    NFE_FORCE_INLINE Vec4f& operator = (const Vec4f& other);

    NFE_FORCE_INLINE static const Vec4f FromInteger(int32 x);
    NFE_FORCE_INLINE static const Vec4f FromIntegers(int32 x, int32 y, int32 z, int32 w);

    NFE_FORCE_INLINE explicit operator float() const { return x; }

    NFE_FORCE_INLINE float operator[] (uint32 index) const
    {
        NFE_ASSERT(index < 4, "Index out of bounds (%u)", index);
        return f[index];
    }

    NFE_FORCE_INLINE float& operator[] (uint32 index)
    {
        NFE_ASSERT(index < 4, "Index out of bounds (%u)", index);
        return f[index];
    }

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE Vec4f(const __m128& src);
    NFE_FORCE_INLINE operator __m128() const { return v; }
    NFE_FORCE_INLINE operator __m128i() const { return _mm_castps_si128(v); }
#endif // NFE_USE_SSE

    // simple arithmetics
    NFE_FORCE_INLINE const Vec4f operator- () const;
    NFE_FORCE_INLINE const Vec4f operator+ (const Vec4f& b) const;
    NFE_FORCE_INLINE const Vec4f operator- (const Vec4f& b) const;
    NFE_FORCE_INLINE const Vec4f operator* (const Vec4f& b) const;
    NFE_FORCE_INLINE const Vec4f operator/ (const Vec4f& b) const;
    NFE_FORCE_INLINE const Vec4f operator* (float b) const;
    NFE_FORCE_INLINE const Vec4f operator/ (float b) const;
    NFE_FORCE_INLINE Vec4f& operator+= (const Vec4f& b);
    NFE_FORCE_INLINE Vec4f& operator-= (const Vec4f& b);
    NFE_FORCE_INLINE Vec4f& operator*= (const Vec4f& b);
    NFE_FORCE_INLINE Vec4f& operator/= (const Vec4f& b);
    NFE_FORCE_INLINE Vec4f& operator*= (float b);
    NFE_FORCE_INLINE Vec4f& operator/= (float b);

    // modulo 1
    NFE_FORCE_INLINE static const Vec4f Mod1(const Vec4f& x);

    NFE_FORCE_INLINE const BoolType operator == (const Vec4f& b) const;
    NFE_FORCE_INLINE const BoolType operator < (const Vec4f& b) const;
    NFE_FORCE_INLINE const BoolType operator <= (const Vec4f& b) const;
    NFE_FORCE_INLINE const BoolType operator > (const Vec4f& b) const;
    NFE_FORCE_INLINE const BoolType operator >= (const Vec4f& b) const;
    NFE_FORCE_INLINE const BoolType operator != (const Vec4f& b) const;

    // bitwise logic operations
    NFE_FORCE_INLINE const Vec4f operator& (const Vec4f& b) const;
    NFE_FORCE_INLINE const Vec4f operator| (const Vec4f& b) const;
    NFE_FORCE_INLINE const Vec4f operator^ (const Vec4f& b) const;
    NFE_FORCE_INLINE Vec4f& operator&= (const Vec4f& b);
    NFE_FORCE_INLINE Vec4f& operator|= (const Vec4f& b);
    NFE_FORCE_INLINE Vec4f& operator^= (const Vec4f& b);

    NFE_FORCE_INLINE const Vec4f SplatX() const;
    NFE_FORCE_INLINE const Vec4f SplatY() const;
    NFE_FORCE_INLINE const Vec4f SplatZ() const;
    NFE_FORCE_INLINE const Vec4f SplatW() const;

    // Change sign of selected elements (immediate)
    template<uint32 flipX, uint32 flipY, uint32 flipZ, uint32 flipW>
    NFE_FORCE_INLINE const Vec4f ChangeSign() const;

    // Change sign of selected elements (variable)
    NFE_FORCE_INLINE const Vec4f ChangeSign(const VecBool4f& flip) const;

    // Prepare mask vector
    template<uint32 maskX, uint32 maskY, uint32 maskZ, uint32 maskW>
    NFE_FORCE_INLINE static const Vec4f MakeMask();

    // Rearrange vector elements (immediate)
    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_FORCE_INLINE const Vec4f Swizzle() const;

    // Rearrange vector elements (variable)
    NFE_FORCE_INLINE const Vec4f Swizzle(uint32 ix, uint32 iy, uint32 iz, uint32 iw) const;

    // shuffle between two vectors
    // returns Vec4f(a[ix], a[iy], b[ix], b[iy])
    template<uint32 ix, uint32 iy, uint32 iz, uint32 iw>
    NFE_FORCE_INLINE static const Vec4f Shuffle(const Vec4f& a, const Vec4f& b);

    // Convert to 3 uint8 values (with clamping)
    // xyz [0.0f...1.0f] -> zyx [0...255]
    NFE_FORCE_INLINE uint32 ToBGR() const;

    // Convert to 4 uint8 values (with clamping)
    NFE_FORCE_INLINE uint32 ToRGBA() const;

    NFE_FORCE_INLINE const Vec2f ToVec2f() const;
    NFE_FORCE_INLINE const Vec3f ToVec3f() const;
    NFE_FORCE_INLINE const Vec4fU ToVec4fU() const;

    NFE_FORCE_INLINE const Half4 ToHalf4() const;

    NFE_FORCE_INLINE static const Vec4f Floor(const Vec4f& v);
    NFE_FORCE_INLINE static const Vec4f Sqrt(const Vec4f& v);
    NFE_FORCE_INLINE static const Vec4f Reciprocal(const Vec4f& v);
    NFE_FORCE_INLINE static const Vec4f FastReciprocal(const Vec4f& v);
    NFE_FORCE_INLINE static const Vec4f Lerp(const Vec4f& v1, const Vec4f& v2, const Vec4f& weight);
    NFE_FORCE_INLINE static const Vec4f Lerp(const Vec4f& v1, const Vec4f& v2, float weight);
    NFE_FORCE_INLINE static const Vec4f Min(const Vec4f& a, const Vec4f& b);
    NFE_FORCE_INLINE static const Vec4f Max(const Vec4f& a, const Vec4f& b);
    NFE_FORCE_INLINE static const Vec4f Abs(const Vec4f& v);
    NFE_FORCE_INLINE static const Vec4f Saturate(const Vec4f& v);
    NFE_FORCE_INLINE static const Vec4f Clamp(const Vec4f& x, const Vec4f& min, const Vec4f& max);

    // Build mask of sign bits
    NFE_FORCE_INLINE uint32 GetSignMask() const;

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise
    NFE_FORCE_INLINE static const Vec4f Select(const Vec4f& a, const Vec4f& b, const BoolType& sel);

    template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
    NFE_FORCE_INLINE static const Vec4f Select(const Vec4f& a, const Vec4f& b);

    // Calculate 2D dot product (scalar result)
    NFE_FORCE_INLINE static float Dot2(const Vec4f& v1, const Vec4f& v2);

    // Calculate 2D dot product (vector result)
    NFE_FORCE_INLINE static const Vec4f Dot2V(const Vec4f& v1, const Vec4f& v2);

    // Calculate 3D dot product (scalar result)
    NFE_FORCE_INLINE static float Dot3(const Vec4f& v1, const Vec4f& v2);

    // Calculate 3D dot product (vector result)
    NFE_FORCE_INLINE static const Vec4f Dot3V(const Vec4f& v1, const Vec4f& v2);

    // Calculate 4D dot product (scalar result)
    NFE_FORCE_INLINE static float Dot4(const Vec4f& v1, const Vec4f& v2);

    // Calculate 4D dot product (vector result)
    NFE_FORCE_INLINE static const Vec4f Dot4V(const Vec4f& v1, const Vec4f& v2);

    // Calculate 3D cross product
    NFE_FORCE_INLINE static const Vec4f Cross3(const Vec4f& v1, const Vec4f& v2);

    // Square length of a 2D vector (scalar result)
    NFE_FORCE_INLINE float SqrLength2() const;

    // Length of a 2D vector (scalar result)
    NFE_FORCE_INLINE float Length2() const;

    // Length of a 2D vector (vector result)
    NFE_FORCE_INLINE const Vec4f Length2V() const;

    // Length of a 3D vector (scalar result)
    NFE_FORCE_INLINE float Length3() const;

    // Square length of a 3D vector (scalar result)
    NFE_FORCE_INLINE float SqrLength3() const;

    // Length of a 3D vector (vector result)
    NFE_FORCE_INLINE const Vec4f Length3V() const;

    // Length of a 4D vector (scalar result)
    NFE_FORCE_INLINE float Length4() const;

    // Length of a 4D vector (vector result)
    NFE_FORCE_INLINE const Vec4f Length4V() const;

    // Square length of a 4D vector (scalar result)
    NFE_FORCE_INLINE float SqrLength4() const;

    // Normalize as 3D vector
    NFE_FORCE_INLINE Vec4f& Normalize3();
    NFE_FORCE_INLINE Vec4f& FastNormalize3();

    // Normalize as 4D vector
    NFE_FORCE_INLINE Vec4f& Normalize4();

    // Return normalized 3D vector
    NFE_FORCE_INLINE const Vec4f Normalized3() const;
    NFE_FORCE_INLINE const Vec4f FastNormalized3() const;

    // Return normalized 3D vector and its inverse
    NFE_FORCE_INLINE const Vec4f InvNormalized(Vec4f& outInvNormalized) const;

    // Return normalized 4D vector
    NFE_FORCE_INLINE const Vec4f Normalized4() const;

    // Reflect a 3D vector
    NFE_FORCE_INLINE static const Vec4f Reflect3(const Vec4f& i, const Vec4f& n);

    // Refract a 3D vector
    NFCOMMON_API static const Vec4f Refract3(const Vec4f& i, const Vec4f& n, float eta);

    // Check if two vectors are (almost) equal
    NFE_FORCE_INLINE static bool AlmostEqual(const Vec4f& v1, const Vec4f& v2, float epsilon = NFE_MATH_EPSILON);

    // Check if the vector is equal to zero
    NFE_FORCE_INLINE const BoolType IsZero() const;

    // Check if any component is NaN
    NFE_FORCE_INLINE const BoolType IsNaN() const;

    // Check if any component is an infinity
    NFE_FORCE_INLINE const BoolType IsInfinite() const;

    // Check if is not NaN or infinity
    NFE_FORCE_INLINE bool IsValid() const;

    // Fused multiply and add (a * b + c)
    NFE_FORCE_INLINE static const Vec4f MulAndAdd(const Vec4f& a, const Vec4f& b, const Vec4f& c);
    NFE_FORCE_INLINE static const Vec4f MulAndAdd(const Vec4f& a, const float b, const Vec4f& c);

    // Fused multiply and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vec4f MulAndSub(const Vec4f& a, const Vec4f& b, const Vec4f& c);
    NFE_FORCE_INLINE static const Vec4f MulAndSub(const Vec4f& a, const float b, const Vec4f& c);

    // Fused multiply (negated) and add (-a * b + c)
    NFE_FORCE_INLINE static const Vec4f NegMulAndAdd(const Vec4f& a, const Vec4f& b, const Vec4f& c);
    NFE_FORCE_INLINE static const Vec4f NegMulAndAdd(const Vec4f& a, const float b, const Vec4f& c);

    // Fused multiply (negated) and subtract (-a * b - c)
    NFE_FORCE_INLINE static const Vec4f NegMulAndSub(const Vec4f& a, const Vec4f& b, const Vec4f& c);
    NFE_FORCE_INLINE static const Vec4f NegMulAndSub(const Vec4f& a, const float b, const Vec4f& c);

    // Calculate horizontal maximum. Result is splatted across all elements
    NFE_FORCE_INLINE const Vec4f HorizontalMax() const;

    // Calculate horizontal sum. Result is splatted across all elements
    NFE_FORCE_INLINE const Vec4f HorizontalSum() const;

    // transpose 3x3 matrix
    NFE_FORCE_INLINE static void Transpose3(Vec4f& a, Vec4f& b, Vec4f& c);

    // make vector "v" orthogonal to "reference" vector
    NFE_FORCE_INLINE static const Vec4f Orthogonalize(const Vec4f& v, const Vec4f& reference);

    // Compute fmodf(x, 1.0f)
    NFE_FORCE_INLINE static const Vec4f Fmod1(const Vec4f& v);
};

// like Vec4f::operator * (float)
NFE_FORCE_INLINE const Vec4f operator*(float a, const Vec4f& b);

// [-1...1] -> [0...1] 
NFE_FORCE_INLINE const Vec4f BipolarToUnipolar(const Vec4f& x);

// [-1...1] -> [0...1] 
NFE_FORCE_INLINE const Vec4f UnipolarToBipolar(const Vec4f& x);

////

// some commonly used constants
NFE_GLOBAL_CONST Vec4f VECTOR_EPSILON = { NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON };
NFE_GLOBAL_CONST Vec4f VECTOR_HALVES = { 0.5f, 0.5f, 0.5f, 0.5f };
NFE_GLOBAL_CONST Vec4f VECTOR_MIN = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
NFE_GLOBAL_CONST Vec4f VECTOR_MAX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
NFE_GLOBAL_CONST Vec4f VECTOR_INF = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
NFE_GLOBAL_CONST Vec4f VECTOR_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_ONE3 = { 1.0f, 1.0f, 1.0f, 0.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_ONE2 = { 1.0f, 1.0f, 0.0f, 0.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_MINUS_ONE = { -1.0f, -1.0f, -1.0f, -1.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_EXPONENT_MASK = { 0x7F800000u, 0x7F800000u, 0x7F800000u, 0x7F800000u };
NFE_GLOBAL_CONST Vec4f VECTOR_MANTISSA_MASK = { 0x007FFFFFu, 0x007FFFFFu, 0x007FFFFFu, 0x007FFFFFu };
NFE_GLOBAL_CONST Vec4f VECTOR_MASK_ABS = { 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu };
NFE_GLOBAL_CONST Vec4f VECTOR_MASK_SIGN_W = { 0u, 0u, 0u, 0x80000000u };
NFE_GLOBAL_CONST Vec4f VECTOR_INV_255 = { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_255 = { 255.0f, 255.0f, 255.0f, 255.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_X = { 1.0f, 0.0f, 0.0f, 0.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_Y = { 0.0f, 1.0f, 0.0f, 0.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_Z = { 0.0f, 0.0f, 1.0f, 0.0f };
NFE_GLOBAL_CONST Vec4f VECTOR_W = { 0.0f, 0.0f, 0.0f, 1.0f };

} // namespace Math
} // namespace NFE


#include "Vec4fImpl.hpp"

#ifdef NFE_USE_SSE
#include "Vec4fImplSSE.hpp"
#else
#include "Vec4fImplNaive.hpp"
#endif // NFE_USE_SSE
