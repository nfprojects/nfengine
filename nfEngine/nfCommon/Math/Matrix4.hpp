#pragma once

#include "Math.hpp"
#include "Vector4.hpp"
#include "Vector3x8.hpp"
#include "Box.hpp"
#include "Ray.hpp"


namespace NFE {
namespace Math {

// 4x4 matrix
class NFE_ALIGN(32) Matrix4 final
{
public:
    union
    {
        Vector4 rows[4];
        Vector8 vec[2];
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
        return { Vector4::Zero(), Vector4::Zero(), Vector4::Zero(), Vector4::Zero() };
    }

    NFE_FORCE_INLINE Matrix4(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3)
    {
        rows[0] = r0;
        rows[1] = r1;
        rows[2] = r2;
        rows[3] = r3;
    }


    NFE_FORCE_INLINE Matrix4(const Vector8& v0, const Vector8& v1)
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

    NFE_FORCE_INLINE Vector4& operator[] (int i)
    {
        return rows[i];
    }

    NFE_FORCE_INLINE const Vector4& operator[] (int i) const
    {
        return rows[i];
    }

    NFE_FORCE_INLINE Vector4& GetRow(int i)
    {
        return rows[i];
    }

    NFE_FORCE_INLINE const Vector4& GetRow(int i) const
    {
        return rows[i];
    }

    NFE_FORCE_INLINE const Vector4& GetTranslation() const
    {
        return rows[3];
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
    NFE_FORCE_INLINE const Vector4 TransformPoint(const Vector4& a) const
    {
        Vector4 t;
        t = Vector4::MulAndAdd(a.SplatX(), rows[0], rows[3]);
        t = Vector4::MulAndAdd(a.SplatY(), rows[1], t);
        t = Vector4::MulAndAdd(a.SplatZ(), rows[2], t);
        return t;
    }

    const Vector3x8 TransformPoint(const Vector3x8& a) const
    {
        const Vector3x8 row0(rows[0]);
        const Vector3x8 row1(rows[1]);
        const Vector3x8 row2(rows[2]);
        const Vector3x8 row3(rows[3]);

        Vector3x8 t;
        t = Vector3x8::MulAndAdd(row0, a.x, row3);
        t = Vector3x8::MulAndAdd(row1, a.y, t);
        t = Vector3x8::MulAndAdd(row2, a.z, t);
        return t;
    }

    NFE_FORCE_INLINE const Vector4 TransformVector(const Vector4& a) const
    {
        Vector4 t = a.SplatX() * rows[0];
        t = Vector4::MulAndAdd(a.SplatY(), rows[1], t);
        t = Vector4::MulAndAdd(a.SplatZ(), rows[2], t);
        return t;
    }

    // transform and negate a vector
    // Note: faster than TransformVector(-a)
    NFE_FORCE_INLINE const Vector4 TransformVectorNeg(const Vector4& a) const
    {
        Vector4 t = a.SplatX() * rows[0];
        t = Vector4::NegMulAndSub(a.SplatY(), rows[1], t);
        t = Vector4::NegMulAndAdd(a.SplatZ(), rows[2], t);
        return t;
    }

    const Vector3x8 TransformVector(const Vector3x8& a) const
    {
        const Vector3x8 row0(rows[0]);
        const Vector3x8 row1(rows[1]);
        const Vector3x8 row2(rows[2]);

        Vector3x8 t = row0 * a.x;
        t = Vector3x8::MulAndAdd(row1, a.y, t);
        t = Vector3x8::MulAndAdd(row2, a.z, t);
        return t;
    }

    // Multiply a 4D vector by a 4x4 matrix.
    // Equivalent of a[0] * m.rows[0] + a[1] * m.rows[1] + a[2] * m.rows[2] + a[3] * m.rows[3]
    NFE_FORCE_INLINE const Vector4 operator * (const Vector4& a) const
    {
        Vector4 t = a.SplatX() * rows[0];
        t = Vector4::MulAndAdd(a.SplatY(), rows[1], t);
        t = Vector4::MulAndAdd(a.SplatZ(), rows[2], t);
        t = Vector4::MulAndAdd(a.SplatW(), rows[3], t);
        return t;
    }

    // Create rotation matrix
    NFCOMMON_API static const Matrix4 MakeRotationNormal(const Vector4& normalAxis, float angle);

    // Create view matrix
    NFCOMMON_API static const Matrix4 MakeLookTo(const Vector4& eyePosition, const Vector4& eyeDirection, const Vector4& upDirection);

    // Create perspective projection matrix
    NFCOMMON_API static const Matrix4 MakePerspective(float aspect, float fovY, float nearZ, float farZ);

    // Create orthographic projection matrix
    NFCOMMON_API static const Matrix4 MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

    // Create matrix representing a translation by 3D vector.
    NFCOMMON_API static const Matrix4 MakeTranslation(const Vector4& pos);

    // Create scaling matrix
    NFCOMMON_API static const Matrix4 MakeScaling(const Vector4& scale);

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
        Vector4::Transpose3(result[0], result[1], result[2]);
        result.rows[3] = result.TransformVectorNeg(rows[3]);
        return result;
    }

    NFE_FORCE_INLINE Matrix4& Transpose()
    {
#ifdef NFE_USE_SSE
        Vector4& row0 = rows[0];
        Vector4& row1 = rows[1];
        Vector4& row2 = rows[2];
        Vector4& row3 = rows[3];
        _MM_TRANSPOSE4_PS(row0, row1, row2, row3);
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
        Vector4 row0 = rows[0];
        Vector4 row1 = rows[1];
        Vector4 row2 = rows[2];
        Vector4 row3 = rows[3];

#ifdef NFE_USE_SSE
        _MM_TRANSPOSE4_PS(row0, row1, row2, row3);
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
        const Vector4 origin = TransformPoint(ray.origin);
        const Vector4 dir = TransformVector(ray.dir);
        return Ray(origin, dir);
    }

    NFE_FORCE_INLINE const Ray TransformRay_Unsafe(const Ray& ray) const
    {
        const Vector4 origin = TransformPoint(ray.origin);
        const Vector4 dir = TransformVector(ray.dir);
        return Ray::BuildUnsafe(origin, dir);
    }
};

} // namespace Math
} // namespace NFE
