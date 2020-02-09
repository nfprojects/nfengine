#include "PCH.hpp"
#include "Matrix4.hpp"
#include "Quaternion.hpp"


namespace NFE {
namespace Math {

const Matrix4 Matrix4::MakeTranslation(const Vector4& pos)
{
    Matrix4 m = Identity();
    m.rows[3] = Vector4(pos.x, pos.y, pos.z, 1.0f);
    return m;
}

const Matrix4 Matrix4::MakeRotationNormal(const Vector4& normalAxis, float angle)
{
    return Quaternion::FromAxisAndAngle(normalAxis, angle).ToMatrix();
}

const Matrix4 Matrix4::MakeLookTo(const Vector4& eyePosition, const Vector4& eyeDirection, const Vector4& upDirection)
{
    Vector4 zaxis = eyeDirection.Normalized3();
    Vector4 xaxis = Vector4::Cross3(upDirection, zaxis).Normalized3();
    Vector4 yaxis = Vector4::Cross3(zaxis, xaxis);

    return Matrix4(Vector4(xaxis.x, yaxis.x, zaxis.x, 0.0f),
                   Vector4(xaxis.y, yaxis.y, zaxis.y, 0.0f),
                   Vector4(xaxis.z, yaxis.z, zaxis.z, 0.0f),
                   Vector4(Vector4::Dot3(xaxis, -eyePosition),
                           Vector4::Dot3(yaxis, -eyePosition),
                           Vector4::Dot3(zaxis, -eyePosition),
                           1.0f));
}

const Matrix4 Matrix4::MakePerspective(float aspect, float fovY, float nearZ, float farZ)
{
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;

    return Matrix4(Vector4(xScale,  0.0f,   0.0f,                           0.0f),
                   Vector4(0.0f,    yScale, 0.0f,                           0.0f),
                   Vector4(0.0f,    0.0f,   farZ / (farZ - nearZ),          1.0f),
                   Vector4(0.0f,    0.0f,   -nearZ * farZ / (farZ - nearZ), 0.0f));
}

const Matrix4 Matrix4::MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    return Matrix4(
               Vector4(2.0f / (right - left), 0.0f,                  0.0f,                  0.0f),
               Vector4(0.0f,                  2.0f / (top - bottom), 0.0f,                  0.0f),
               Vector4(0.0f,                  0.0f,                  1.0f / (zFar - zNear), 0.0f),
               Vector4((left + right) / (left - right),
                       (top + bottom) / (bottom - top),
                       zNear / (zNear - zFar),
                       1.0f));
}

const Matrix4 Matrix4::MakeScaling(const Vector4& scale)
{
    Matrix4 m = Identity();
    m.rows[0] *= scale.SplatX();
    m.rows[1] *= scale.SplatY();
    m.rows[2] *= scale.SplatZ();
    return m;
}

const Matrix4 Matrix4::operator * (const Matrix4& b) const
{
#ifdef NFE_USE_AVX

    // based on:
    // https://stackoverflow.com/a/46108376/10061517

    Vector8 a0, a1, b0, b1;
    Vector8 c0, c1;

    b0 = _mm256_permute2f128_ps(b.vec[0], b.vec[0], 0x00);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(0, 0, 0, 0));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(0, 0, 0, 0));
    c0 = a0 * b0;
    c1 = a1 * b0;

    b0 = _mm256_permute2f128_ps(b.vec[0], b.vec[0], 0x11);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(1, 1, 1, 1));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(1, 1, 1, 1));
    c0 = Vector8::MulAndAdd(a0, b0, c0);
    c1 = Vector8::MulAndAdd(a1, b0, c1);

    b1 = _mm256_permute2f128_ps(b.vec[1], b.vec[1], 0x00);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(2, 2, 2, 2));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(2, 2, 2, 2));
    c0 = Vector8::MulAndAdd(a0, b1, c0);
    c1 = Vector8::MulAndAdd(a1, b1, c1);

    b1 = _mm256_permute2f128_ps(b.vec[1], b.vec[1], 0x11);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(3, 3, 3, 3));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(3, 3, 3, 3));
    c0 = Vector8::MulAndAdd(a0, b1, c0);
    c1 = Vector8::MulAndAdd(a1, b1, c1);

    return Matrix4{ c0, c1 };

#else

    return Matrix4{ b * rows[0], b * rows[1], b * rows[2], b * rows[3] };

#endif
}

Matrix4& Matrix4::operator *= (const Matrix4& b)
{
    rows[0] = b * rows[0];
    rows[1] = b * rows[1];
    rows[2] = b * rows[2];
    rows[3] = b * rows[3];
    return *this;
}

float Matrix4::Determinant() const
{
    Vector4 v0 = rows[2].Swizzle<1, 0, 0, 0>();
    Vector4 v1 = rows[3].Swizzle<2, 2, 1, 1>();
    Vector4 v2 = rows[2].Swizzle<1, 0, 0, 0>();
    Vector4 v3 = rows[3].Swizzle<3, 3, 3, 2>();
    Vector4 v4 = rows[2].Swizzle<2, 2, 1, 1>();
    Vector4 v5 = rows[3].Swizzle<3, 3, 3, 2>();

    Vector4 p0 = v0 * v1;
    Vector4 p1 = v2 * v3;
    Vector4 p2 = v4 * v5;

    v0 = rows[2].Swizzle<2, 2, 1, 1>();
    v1 = rows[3].Swizzle<1, 0, 0, 0>();
    v2 = rows[2].Swizzle<3, 3, 3, 2>();
    v3 = rows[3].Swizzle<1, 0, 0, 0>();
    v4 = rows[2].Swizzle<3, 3, 3, 2>();
    v5 = rows[3].Swizzle<2, 2, 1, 1>();

    p0 = Vector4::NegMulAndAdd(v0, v1, p0);
    p1 = Vector4::NegMulAndAdd(v2, v3, p1);
    p2 = Vector4::NegMulAndAdd(v4, v5, p2);

    v0 = rows[1].Swizzle<3, 3, 3, 2>();
    v1 = rows[1].Swizzle<2, 2, 1, 1>();
    v2 = rows[1].Swizzle<1, 0, 0, 0>();

    Vector4 r = v0 * p0;
    r = Vector4::NegMulAndAdd(v1, p1, r);
    r = Vector4::MulAndAdd(v2, p2, r);

    Vector4 s = rows[0] * Vector4{ 1.0f, -1.0f, 1.0f, -1.0f };
    return Vector4::Dot4(s, r);
}

// for row major matrix
// we use Vector4 to represent 2x2 matrix as A = | A0  A1 |
//                                               | A2  A3 |
// 2x2 row major Matrix multiply A*B
NFE_FORCE_INLINE static const Vector4 Mat2Mul(const Vector4 vec1, const Vector4 vec2)
{
    return vec1 * vec2.Swizzle<0, 3, 0, 3>() + vec1.Swizzle<1, 0, 3, 2>() * vec2.Swizzle<2, 1, 2, 1>();
}
// 2x2 row major Matrix adjugate multiply (A#)*B
NFE_FORCE_INLINE static const Vector4 Mat2AdjMul(const Vector4 vec1, const Vector4 vec2)
{
    return vec1.Swizzle<3, 3, 0, 0>() * vec2 - vec1.Swizzle<1, 1, 2, 2>() * vec2.Swizzle<2, 3, 0, 1>();

}
// 2x2 row major Matrix multiply adjugate A*(B#)
NFE_FORCE_INLINE static const Vector4 Mat2MulAdj(const Vector4 vec1, const Vector4 vec2)
{
    return vec1 * vec2.Swizzle<3, 0, 3, 0>() - vec1.Swizzle<1, 0, 3, 2>() * vec2.Swizzle<2, 1, 2, 1>();
}

const Matrix4 Matrix4::Inverted() const
{
    // based on:
    // https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

    const Vector4 A = Vector4::Shuffle<0,1,0,1>(rows[0], rows[1]);
    const Vector4 B = Vector4::Shuffle<2,3,2,3>(rows[0], rows[1]);
    const Vector4 C = Vector4::Shuffle<0,1,0,1>(rows[2], rows[3]);
    const Vector4 D = Vector4::Shuffle<2,3,2,3>(rows[2], rows[3]);

    // determinant as (|A| |B| |C| |D|)
    const Vector4 detSub =
        Vector4::Shuffle<0,2,0,2>(rows[0], rows[2]) * Vector4::Shuffle<1,3,1,3>(rows[1], rows[3]) -
        Vector4::Shuffle<1,3,1,3>(rows[0], rows[2]) * Vector4::Shuffle<0,2,0,2>(rows[1], rows[3]);

    const Vector4 detA = detSub.SplatX();
    const Vector4 detB = detSub.SplatY();
    const Vector4 detC = detSub.SplatZ();
    const Vector4 detD = detSub.SplatW();

    const Vector4 D_C = Mat2AdjMul(D, C); // D#C
    const Vector4 A_B = Mat2AdjMul(A, B); // A#B

    // tr((A#B)(D#C))
    const Vector4 tr = (A_B * D_C.Swizzle<0, 2, 1, 3>()).HorizontalSum();

    // |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
    const Vector4 detM = detA * detD + detB * detC - tr;

    // (1/|M|, -1/|M|, -1/|M|, 1/|M|)
    const Vector4 rDetM = Vector4(1.0f, -1.0f, -1.0f, 1.0f) / detM;

    Vector4 X = rDetM * Vector4::MulAndSub(detD, A, Mat2Mul(B, D_C));       // X# = |D|A - B(D#C)
    Vector4 W = rDetM * Vector4::MulAndSub(detA, D, Mat2Mul(C, A_B));       // W# = |A|D - C(A#B)
    Vector4 Y = rDetM * Vector4::MulAndSub(detB, C, Mat2MulAdj(D, A_B));    // Y# = |B|C - D(A#B)#
    Vector4 Z = rDetM * Vector4::MulAndSub(detC, B, Mat2MulAdj(A, D_C));    // Z# = |C|B - A(D#C)#

    Matrix4 r;

    // apply adjugate and store, here we combine adjugate shuffle and store shuffle
    r.rows[0] = Vector4::Shuffle<3,1,3,1>(X, Y);
    r.rows[1] = Vector4::Shuffle<2,0,2,0>(X, Y);
    r.rows[2] = Vector4::Shuffle<3,1,3,1>(Z, W);
    r.rows[3] = Vector4::Shuffle<2,0,2,0>(Z, W);

    return r;
}

const Box Matrix4::TransformBox(const Box& box) const
{
    // based on:
    // http://dev.theomader.com/transform-bounding-boxes/

    const Vector4 xa = rows[0] * box.min.x;
    const Vector4 xb = rows[0] * box.max.x;
    const Vector4 ya = rows[1] * box.min.y;
    const Vector4 yb = rows[1] * box.max.y;
    const Vector4 za = rows[2] * box.min.z;
    const Vector4 zb = rows[2] * box.max.z;

    return Box(
        Vector4::Min(xa, xb) + Vector4::Min(ya, yb) + Vector4::Min(za, zb) + rows[3],
        Vector4::Max(xa, xb) + Vector4::Max(ya, yb) + Vector4::Max(za, zb) + rows[3]
    );
}

} // namespace Math
} // namespace NFE
