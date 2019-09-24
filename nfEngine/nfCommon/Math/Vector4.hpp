#pragma once

#include "Math.hpp"

#include "Float2.hpp"
#include "Float3.hpp"
#include "Float4.hpp"
#include "VectorBool4.hpp"

namespace NFE {
namespace Math {

/**
 * 4-element SIMD vector
 */
struct NFE_ALIGN(16) Vector4
{
    union
    {
        float f[4];
        int32 i[4];
        uint32 u[4];

#ifdef NFE_USE_SSE
        __m128 v;
        __m128i vi;
#endif // NFE_USE_SSE

        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };

    NFE_FORCE_INLINE Vector4();
    NFE_FORCE_INLINE Vector4(const Vector4& other);
    NFE_FORCE_INLINE static const Vector4 Zero();
    NFE_FORCE_INLINE explicit Vector4(const float scalar); // splat
    NFE_FORCE_INLINE explicit Vector4(const int32 scalar); // splat
    NFE_FORCE_INLINE explicit Vector4(const uint32 scalar); // splat
    NFE_FORCE_INLINE Vector4(const float x, const float y, const float z = 0.0f, const float w = 0.0f);
    NFE_FORCE_INLINE Vector4(const int32 x, const int32 y, const int32 z = 0, const int32 w = 0);
    NFE_FORCE_INLINE Vector4(const uint32 x, const uint32 y, const uint32 z = 0u, const uint32 w = 0u);
    NFE_FORCE_INLINE explicit Vector4(const float* src);
    NFE_FORCE_INLINE explicit Vector4(const Float2& src);
    NFE_FORCE_INLINE explicit Vector4(const Float3& src);
    NFE_FORCE_INLINE explicit Vector4(const Float4& src);
    NFE_FORCE_INLINE Vector4& operator = (const Vector4& other);

    NFE_FORCE_INLINE static const Vector4 FromInteger(int32 x);
    NFE_FORCE_INLINE static const Vector4 FromIntegers(int32 x, int32 y, int32 z, int32 w);

    NFE_FORCE_INLINE explicit operator float() const { return x; }
    NFE_FORCE_INLINE float operator[] (uint32 index) const { return f[index]; }
    NFE_FORCE_INLINE float& operator[] (uint32 index) { return f[index]; }

#ifdef NFE_USE_SSE
    NFE_FORCE_INLINE Vector4(const __m128& src);
    NFE_FORCE_INLINE operator __m128() const { return v; }
    NFE_FORCE_INLINE operator __m128i() const { return _mm_castps_si128(v); }
#endif // NFE_USE_SSE

    // simple arithmetics
    NFE_FORCE_INLINE const Vector4 operator- () const;
    NFE_FORCE_INLINE const Vector4 operator+ (const Vector4& b) const;
    NFE_FORCE_INLINE const Vector4 operator- (const Vector4& b) const;
    NFE_FORCE_INLINE const Vector4 operator* (const Vector4& b) const;
    NFE_FORCE_INLINE const Vector4 operator/ (const Vector4& b) const;
    NFE_FORCE_INLINE const Vector4 operator* (float b) const;
    NFE_FORCE_INLINE const Vector4 operator/ (float b) const;
    NFE_FORCE_INLINE Vector4& operator+= (const Vector4& b);
    NFE_FORCE_INLINE Vector4& operator-= (const Vector4& b);
    NFE_FORCE_INLINE Vector4& operator*= (const Vector4& b);
    NFE_FORCE_INLINE Vector4& operator/= (const Vector4& b);
    NFE_FORCE_INLINE Vector4& operator*= (float b);
    NFE_FORCE_INLINE Vector4& operator/= (float b);

    // modulo 1
    NFE_FORCE_INLINE static const Vector4 Mod1(const Vector4& x);

    NFE_FORCE_INLINE const VectorBool4 operator == (const Vector4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator < (const Vector4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator <= (const Vector4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator > (const Vector4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator >= (const Vector4& b) const;
    NFE_FORCE_INLINE const VectorBool4 operator != (const Vector4& b) const;

    // bitwise logic operations
    NFE_FORCE_INLINE const Vector4 operator& (const Vector4& b) const;
    NFE_FORCE_INLINE const Vector4 operator| (const Vector4& b) const;
    NFE_FORCE_INLINE const Vector4 operator^ (const Vector4& b) const;
    NFE_FORCE_INLINE Vector4& operator&= (const Vector4& b);
    NFE_FORCE_INLINE Vector4& operator|= (const Vector4& b);
    NFE_FORCE_INLINE Vector4& operator^= (const Vector4& b);

    NFE_FORCE_INLINE const Vector4 SplatX() const;
    NFE_FORCE_INLINE const Vector4 SplatY() const;
    NFE_FORCE_INLINE const Vector4 SplatZ() const;
    NFE_FORCE_INLINE const Vector4 SplatW() const;

    // Change sign of selected elements (immediate)
    template<uint32 flipX, uint32 flipY, uint32 flipZ, uint32 flipW>
    NFE_FORCE_INLINE const Vector4 ChangeSign() const;

    // Change sign of selected elements (variable)
    NFE_FORCE_INLINE const Vector4 ChangeSign(const VectorBool4& flip) const;

    // Prepare mask vector
    template<uint32 maskX, uint32 maskY, uint32 maskZ, uint32 maskW>
    NFE_FORCE_INLINE static const Vector4 MakeMask();

    // Rearrange vector elements (immediate)
    template<uint32 ix = 0, uint32 iy = 1, uint32 iz = 2, uint32 iw = 3>
    NFE_FORCE_INLINE const Vector4 Swizzle() const;

    // Rearrange vector elements (variable)
    NFE_FORCE_INLINE const Vector4 Swizzle(uint32 ix, uint32 iy, uint32 iz, uint32 iw) const;

    // Convert to 3 uint8 values (with clamping)
    // xyz [0.0f...1.0f] -> zyx [0...255]
    NFE_FORCE_INLINE uint32 ToBGR() const;

    // Convert to 4 uint8 values (with clamping)
    NFE_FORCE_INLINE uint32 ToRGBA() const;

    NFE_FORCE_INLINE const Float2 ToFloat2() const;
    NFE_FORCE_INLINE const Float3 ToFloat3() const;
    NFE_FORCE_INLINE const Float4 ToFloat4() const;

    NFE_FORCE_INLINE static const Vector4 Floor(const Vector4& v);
    NFE_FORCE_INLINE static const Vector4 Sqrt(const Vector4& v);
    NFE_FORCE_INLINE static const Vector4 Reciprocal(const Vector4& v);
    NFE_FORCE_INLINE static const Vector4 FastReciprocal(const Vector4& v);
    NFE_FORCE_INLINE static const Vector4 Lerp(const Vector4& v1, const Vector4& v2, const Vector4& weight);
    NFE_FORCE_INLINE static const Vector4 Lerp(const Vector4& v1, const Vector4& v2, float weight);
    NFE_FORCE_INLINE static const Vector4 Min(const Vector4& a, const Vector4& b);
    NFE_FORCE_INLINE static const Vector4 Max(const Vector4& a, const Vector4& b);
    NFE_FORCE_INLINE static const Vector4 Abs(const Vector4& v);
    NFE_FORCE_INLINE static const Vector4 Saturate(const Vector4& v);
    NFE_FORCE_INLINE static const Vector4 Clamp(const Vector4& x, const Vector4& min, const Vector4& max);

    // Build mask of sign bits
    NFE_FORCE_INLINE int GetSignMask() const;

    // For each vector component, copy value from "a" if "sel" is "false", or from "b" otherwise
    NFE_FORCE_INLINE static const Vector4 Select(const Vector4& a, const Vector4& b, const VectorBool4& sel);

    template<uint32 selX, uint32 selY, uint32 selZ, uint32 selW>
    NFE_FORCE_INLINE static const Vector4 Select(const Vector4& a, const Vector4& b);

    // Calculate 2D dot product (scalar result)
    NFE_FORCE_INLINE static float Dot2(const Vector4& v1, const Vector4& v2);

    // Calculate 2D dot product (vector result)
    NFE_FORCE_INLINE static const Vector4 Dot2V(const Vector4& v1, const Vector4& v2);

    // Calculate 3D dot product (scalar result)
    NFE_FORCE_INLINE static float Dot3(const Vector4& v1, const Vector4& v2);

    // Calculate 3D dot product (vector result)
    NFE_FORCE_INLINE static const Vector4 Dot3V(const Vector4& v1, const Vector4& v2);

    // Calculate 4D dot product (scalar result)
    NFE_FORCE_INLINE static float Dot4(const Vector4& v1, const Vector4& v2);

    // Calculate 4D dot product (vector result)
    NFE_FORCE_INLINE static const Vector4 Dot4V(const Vector4& v1, const Vector4& v2);

    // Calculate 3D cross product
    NFE_FORCE_INLINE static const Vector4 Cross3(const Vector4& v1, const Vector4& v2);

    // Square length of a 2D vector (scalar result)
    NFE_FORCE_INLINE float SqrLength2() const;

    // Length of a 2D vector (scalar result)
    NFE_FORCE_INLINE float Length2() const;

    // Length of a 2D vector (vector result)
    NFE_FORCE_INLINE const Vector4 Length2V() const;

    // Length of a 3D vector (scalar result)
    NFE_FORCE_INLINE float Length3() const;

    // Square length of a 3D vector (scalar result)
    NFE_FORCE_INLINE float SqrLength3() const;

    // Length of a 3D vector (vector result)
    NFE_FORCE_INLINE const Vector4 Length3V() const;

    // Length of a 4D vector (scalar result)
    NFE_FORCE_INLINE float Length4() const;

    // Length of a 4D vector (vector result)
    NFE_FORCE_INLINE const Vector4 Length4V() const;

    // Square length of a 4D vector (scalar result)
    NFE_FORCE_INLINE float SqrLength4() const;

    // Normalize as 3D vector
    NFE_FORCE_INLINE Vector4& Normalize3();
    NFE_FORCE_INLINE Vector4& FastNormalize3();

    // Normalize as 4D vector
    NFE_FORCE_INLINE Vector4& Normalize4();

    // Return normalized 3D vector
    NFE_FORCE_INLINE const Vector4 Normalized3() const;
    NFE_FORCE_INLINE const Vector4 FastNormalized3() const;

    // Return normalized 3D vector and its inverse
    NFE_FORCE_INLINE const Vector4 InvNormalized(Vector4& outInvNormalized) const;

    // Return normalized 4D vector
    NFE_FORCE_INLINE const Vector4 Normalized4() const;

    // Reflect a 3D vector
    NFE_FORCE_INLINE static const Vector4 Reflect3(const Vector4& i, const Vector4& n);

    // Refract a 3D vector
    NFCOMMON_API static const Vector4 Refract3(const Vector4& i, const Vector4& n, float eta);

    // Check if two vectors are (almost) equal
    NFE_FORCE_INLINE static bool AlmostEqual(const Vector4& v1, const Vector4& v2, float epsilon = NFE_MATH_EPSILON);

    // Check if the vector is equal to zero
    NFE_FORCE_INLINE const VectorBool4 IsZero() const;

    // Check if any component is NaN
    NFE_FORCE_INLINE const VectorBool4 IsNaN() const;

    // Check if any component is an infinity
    NFE_FORCE_INLINE const VectorBool4 IsInfinite() const;

    // Check if is not NaN or infinity
    NFE_FORCE_INLINE bool IsValid() const;

    // Fused multiply and add (a * b + c)
    NFE_FORCE_INLINE static const Vector4 MulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);
    NFE_FORCE_INLINE static const Vector4 MulAndAdd(const Vector4& a, const float b, const Vector4& c);

    // Fused multiply and subtract (a * b - c)
    NFE_FORCE_INLINE static const Vector4 MulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);
    NFE_FORCE_INLINE static const Vector4 MulAndSub(const Vector4& a, const float b, const Vector4& c);

    // Fused multiply (negated) and add (-a * b + c)
    NFE_FORCE_INLINE static const Vector4 NegMulAndAdd(const Vector4& a, const Vector4& b, const Vector4& c);
    NFE_FORCE_INLINE static const Vector4 NegMulAndAdd(const Vector4& a, const float b, const Vector4& c);

    // Fused multiply (negated) and subtract (-a * b - c)
    NFE_FORCE_INLINE static const Vector4 NegMulAndSub(const Vector4& a, const Vector4& b, const Vector4& c);
    NFE_FORCE_INLINE static const Vector4 NegMulAndSub(const Vector4& a, const float b, const Vector4& c);

    // Calculate horizontal maximum. Result is splatted across all elements
    NFE_FORCE_INLINE const Vector4 HorizontalMax() const;

    // transpose 3x3 matrix
    NFE_FORCE_INLINE static void Transpose3(Vector4& a, Vector4& b, Vector4& c);

    // make vector "v" orthogonal to "reference" vector
    NFE_FORCE_INLINE static const Vector4 Orthogonalize(const Vector4& v, const Vector4& reference);

    // Compute fmodf(x, 1.0f)
    NFE_FORCE_INLINE static const Vector4 Fmod1(const Vector4& v);
};

// like Vector4::operator * (float)
NFE_FORCE_INLINE const Vector4 operator*(float a, const Vector4& b);

// [-1...1] -> [0...1] 
NFE_FORCE_INLINE const Vector4 BipolarToUnipolar(const Vector4& x);

// [-1...1] -> [0...1] 
NFE_FORCE_INLINE const Vector4 UnipolarToBipolar(const Vector4& x);

////

// some commonly used constants
NFE_GLOBAL_CONST Vector4 VECTOR_EPSILON = { NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON };
NFE_GLOBAL_CONST Vector4 VECTOR_HALVES = { 0.5f, 0.5f, 0.5f, 0.5f };
NFE_GLOBAL_CONST Vector4 VECTOR_MIN = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
NFE_GLOBAL_CONST Vector4 VECTOR_MAX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
NFE_GLOBAL_CONST Vector4 VECTOR_INF = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
NFE_GLOBAL_CONST Vector4 VECTOR_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_ONE3 = { 1.0f, 1.0f, 1.0f, 0.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_ONE2 = { 1.0f, 1.0f, 0.0f, 0.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_MINUS_ONE = { -1.0f, -1.0f, -1.0f, -1.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_EXPONENT_MASK = { 0x7F800000u, 0x7F800000u, 0x7F800000u, 0x7F800000u };
NFE_GLOBAL_CONST Vector4 VECTOR_MANTISSA_MASK = { 0x007FFFFFu, 0x007FFFFFu, 0x007FFFFFu, 0x007FFFFFu };
NFE_GLOBAL_CONST Vector4 VECTOR_MASK_ABS = { 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu, 0x7FFFFFFFu };
NFE_GLOBAL_CONST Vector4 VECTOR_MASK_SIGN_W = { 0u, 0u, 0u, 0x80000000u };
NFE_GLOBAL_CONST Vector4 VECTOR_INV_255 = { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_255 = { 255.0f, 255.0f, 255.0f, 255.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_X = { 1.0f, 0.0f, 0.0f, 0.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_Y = { 0.0f, 1.0f, 0.0f, 0.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_Z = { 0.0f, 0.0f, 1.0f, 0.0f };
NFE_GLOBAL_CONST Vector4 VECTOR_W = { 0.0f, 0.0f, 0.0f, 1.0f };

} // namespace Math
} // namespace NFE


#include "Vector4Impl.hpp"

#ifdef NFE_USE_SSE
#include "Vector4ImplSSE.hpp"
#else
#include "Vector4ImplNaive.hpp"
#endif // NFE_USE_SSE
