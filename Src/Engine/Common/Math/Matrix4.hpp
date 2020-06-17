#pragma once

#include "Math.hpp"
#include "Vec8f.hpp"
#include "Box.hpp"
#include "Ray.hpp"


namespace NFE {
namespace Math {

// 4x4 matrix
class NFE_ALIGN(32) Matrix4 final
{
public:
    NFE_ALIGNED_CLASS(16)

    union
    {
        Vec4f rows[4];
        Vec8f vec[2];
        float f[16];
        float m[4][4];
    };

    NFE_FORCE_INLINE Matrix4() { }

    NFE_FORCE_INLINE static const Matrix4 Identity()
    {
        return { VECTOR_X, VECTOR_Y, VECTOR_Z, VECTOR_W };
    }

    NFE_FORCE_INLINE static const Matrix4 Zero()
    {
        return { Vec4f::Zero(), Vec4f::Zero(), Vec4f::Zero(), Vec4f::Zero() };
    }

    NFE_FORCE_INLINE Matrix4(const Vec4f& r0, const Vec4f& r1, const Vec4f& r2, const Vec4f& r3)
    {
        rows[0] = r0;
        rows[1] = r1;
        rows[2] = r2;
        rows[3] = r3;
    }

    NFE_FORCE_INLINE Matrix4(const Vec8f& v0, const Vec8f& v1)
    {
        vec[0] = v0;
        vec[1] = v1;
    }

    NFE_FORCE_INLINE Matrix4(const Matrix4& other)
    {
        rows[0] = other.rows[0];
        rows[1] = other.rows[1];
        rows[2] = other.rows[2];
        rows[3] = other.rows[3];
    }

    NFE_FORCE_INLINE Matrix4& operator = (const Matrix4& other)
    {
        rows[0] = other.rows[0];
        rows[1] = other.rows[1];
        rows[2] = other.rows[2];
        rows[3] = other.rows[3];
        return *this;
    }

    NFE_FORCE_INLINE Vec4f& operator[] (int i)
    {
        return rows[i];
    }

    NFE_FORCE_INLINE const Vec4f& operator[] (int i) const
    {
        return rows[i];
    }

    NFE_FORCE_INLINE Vec4f& GetRow(int i)
    {
        return rows[i];
    }

    NFE_FORCE_INLINE const Vec4f& GetRow(int i) const
    {
        return rows[i];
    }

    NFE_FORCE_INLINE const Vec4f& GetTranslation() const
    {
        return rows[3];
    }

    NFE_FORCE_INLINE const Matrix4 operator - (const Matrix4& other) const
    {
        return
        {
            rows[0] - other.rows[0],
            rows[1] - other.rows[1],
            rows[2] - other.rows[2],
            rows[3] - other.rows[3],
        };
    }

    NFE_FORCE_INLINE const Matrix4 Abs() const
    {
        return
        {
            Vec4f::Abs(rows[0]),
            Vec4f::Abs(rows[1]),
            Vec4f::Abs(rows[2]),
            Vec4f::Abs(rows[3]),
        };
    }

    NFE_FORCE_INLINE float Max() const
    {
        const Vec4f rowsMax = Vec4f::Max(Vec4f::Max(rows[0], rows[1]), Vec4f::Max(rows[2], rows[3]));
        return rowsMax.HorizontalMax().x;
    }

    // Compute matrix determinant
    NFCOMMON_API float Determinant() const;

    // Matrix multiplication
    NFCOMMON_API const Matrix4 operator * (const Matrix4& b) const;
    NFCOMMON_API Matrix4& operator *= (const Matrix4& b);

    NFE_FORCE_INLINE bool IsValid() const
    {
        return rows[0].IsValid() && rows[1].IsValid() && rows[2].IsValid() && rows[3].IsValid();
    }

    // Returns true if all the corresponding elements are equal.
    NFE_FORCE_INLINE bool operator == (const Matrix4& b) const
    {
        bool tmp0 = (rows[0] == b.rows[0]).All();
        bool tmp1 = (rows[1] == b.rows[1]).All();
        bool tmp2 = (rows[2] == b.rows[2]).All();
        bool tmp3 = (rows[3] == b.rows[3]).All();
        return (tmp0 && tmp1) & (tmp2 && tmp3);
    }

    // Multiply a 3D vector by a 4x4 matrix (affine transform).
    // Equivalent of a[0] * m.rows[0] + a[1] * m.rows[1] + a[2] * m.rows[2] + m.rows[3]
    template<typename T>
    NFE_FORCE_INLINE const T TransformPoint(const T& a) const
    {
        T t;
        t = T::MulAndAdd(T(rows[0]), a.x, T(rows[3]));
        t = T::MulAndAdd(T(rows[1]), a.y, t);
        t = T::MulAndAdd(T(rows[2]), a.z, t);
        return t;
    }

    template<typename T>
    NFE_FORCE_INLINE const T TransformVector(const T& a) const
    {
        T t = T(rows[0]) * a.x;
        t = T::MulAndAdd(T(rows[1]), a.y, t);
        t = T::MulAndAdd(T(rows[2]), a.z, t);
        return t;
    }

    // transform and negate a vector
    // Note: faster than TransformVector(-a)
    template<typename T>
    NFE_FORCE_INLINE const T TransformVectorNeg(const T& a) const
    {
        T t = T(rows[0]) * a.x;
        t = T::NegMulAndSub(T(rows[1]), a.y, t);
        t = T::NegMulAndAdd(T(rows[2]), a.z, t);
        return t;
    }

    // Multiply a 4D vector by a 4x4 matrix.
    // Equivalent of a[0] * m.rows[0] + a[1] * m.rows[1] + a[2] * m.rows[2] + a[3] * m.rows[3]
    NFE_FORCE_INLINE const Vec4f operator * (const Vec4f& a) const
    {
        Vec4f t = a.SplatX() * rows[0];
        t = Vec4f::MulAndAdd(a.SplatY(), rows[1], t);
        t = Vec4f::MulAndAdd(a.SplatZ(), rows[2], t);
        t = Vec4f::MulAndAdd(a.SplatW(), rows[3], t);
        return t;
    }

    // Create rotation matrix
    NFCOMMON_API static const Matrix4 MakeRotationNormal(const Vec4f& normalAxis, float angle);

    // Create view matrix
    NFCOMMON_API static const Matrix4 MakeLookTo(const Vec4f& eyePosition, const Vec4f& eyeDirection, const Vec4f& upDirection);

    // Create perspective projection matrix
    NFCOMMON_API static const Matrix4 MakePerspective(float aspect, float fovY, float nearZ, float farZ);

    // Create orthographic projection matrix
    NFCOMMON_API static const Matrix4 MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

    // Create matrix representing a translation by 3D vector.
    NFCOMMON_API static const Matrix4 MakeTranslation(const Vec4f& pos);

    // Create scaling matrix
    NFCOMMON_API static const Matrix4 MakeScaling(const Vec4f& scale);

    // Get inverted matrix
    NFCOMMON_API const Matrix4 Inverted() const;

    // Invert this matrix
    NFE_FORCE_INLINE Matrix4& Invert()
    {
        *this = Inverted();
        return *this;
    }

    NFE_FORCE_INLINE const Matrix4 FastInverseNoScale() const
    {
        Matrix4 result = *this;
        result.rows[3] = VECTOR_W;
        Vec4f::Transpose3(result[0], result[1], result[2]);
        result.rows[3] = result.TransformVectorNeg(rows[3]);
        return result;
    }

    NFE_FORCE_INLINE Matrix4& Transpose()
    {
#ifdef NFE_USE_SSE
        const __m128 t0 = _mm_unpacklo_ps(rows[0], rows[1]);
        const __m128 t1 = _mm_unpacklo_ps(rows[2], rows[3]);
        const __m128 t2 = _mm_unpackhi_ps(rows[0], rows[1]);
        const __m128 t3 = _mm_unpackhi_ps(rows[2], rows[3]);
        rows[0] = _mm_movelh_ps(t0, t1);
        rows[1] = _mm_movehl_ps(t1, t0);
        rows[2] = _mm_movelh_ps(t2, t3);
        rows[3] = _mm_movehl_ps(t3, t2);
#else // !NFE_USE_SSE
        std::swap(rows[0][1], rows[1][0]);
        std::swap(rows[0][2], rows[2][0]);
        std::swap(rows[0][3], rows[3][0]);
        std::swap(rows[1][2], rows[2][1]);
        std::swap(rows[1][3], rows[3][1]);
        std::swap(rows[2][3], rows[3][2]);
#endif // NFE_USE_SSE

        return *this;
    }

    NFE_FORCE_INLINE const Matrix4 Transposed() const
    {
        Vec4f row0 = rows[0];
        Vec4f row1 = rows[1];
        Vec4f row2 = rows[2];
        Vec4f row3 = rows[3];

#ifdef NFE_USE_SSE
        const __m128 t0 = _mm_unpacklo_ps(row0, row1);
        const __m128 t1 = _mm_unpacklo_ps(row2, row3);
        const __m128 t2 = _mm_unpackhi_ps(row0, row1);
        const __m128 t3 = _mm_unpackhi_ps(row2, row3);
        row0 = _mm_movelh_ps(t0, t1);
        row1 = _mm_movehl_ps(t1, t0);
        row2 = _mm_movelh_ps(t2, t3);
        row3 = _mm_movehl_ps(t3, t2);
#else // !NFE_USE_SSE
        std::swap(row0[1], row1[0]);
        std::swap(row0[2], row2[0]);
        std::swap(row0[3], row3[0]);
        std::swap(row1[2], row2[1]);
        std::swap(row1[3], row3[1]);
        std::swap(row2[3], row3[2]);
#endif // NFE_USE_SSE

        return Matrix4(row0, row1, row2, row3);
    }

    NFCOMMON_API const Box TransformBox(const Box& box) const;

    NFE_FORCE_INLINE const Ray TransformRay(const Ray& ray) const
    {
        const Vec4f origin = TransformPoint(ray.origin);
        const Vec4f dir = TransformVector(ray.dir);
        return Ray(origin, dir);
    }

    NFE_FORCE_INLINE const Ray TransformRay_Unsafe(const Ray& ray) const
    {
        const Vec4f origin = TransformPoint(ray.origin);
        const Vec4f dir = TransformVector(ray.dir);
        return Ray::BuildUnsafe(origin, dir);
    }
};


static_assert(sizeof(Matrix4) == sizeof(float) * 16, "Invalid sizeof Matrix4");


} // namespace Math
} // namespace NFE
