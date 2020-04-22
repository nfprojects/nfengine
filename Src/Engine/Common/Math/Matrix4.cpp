#include "PCH.hpp"
#include "Matrix4.hpp"
#include "Vec8f.hpp"
#include "Quaternion.hpp"


namespace NFE {
namespace Math {

const Matrix4 Matrix4::MakeTranslation(const Vec4f& pos)
{
    Matrix4 m = Identity();
    m.rows[3] = Vec4f(pos.x, pos.y, pos.z, 1.0f);
    return m;
}

const Matrix4 Matrix4::MakeRotationNormal(const Vec4f& normalAxis, float angle)
{
    return Quaternion::FromAxisAndAngle(normalAxis, angle).ToMatrix();
}

const Matrix4 Matrix4::MakeLookTo(const Vec4f& eyePosition, const Vec4f& eyeDirection, const Vec4f& upDirection)
{
    Vec4f zaxis = eyeDirection.Normalized3();
    Vec4f xaxis = Vec4f::Cross3(upDirection, zaxis).Normalized3();
    Vec4f yaxis = Vec4f::Cross3(zaxis, xaxis);

    return Matrix4(Vec4f(xaxis.x, yaxis.x, zaxis.x, 0.0f),
                   Vec4f(xaxis.y, yaxis.y, zaxis.y, 0.0f),
                   Vec4f(xaxis.z, yaxis.z, zaxis.z, 0.0f),
                   Vec4f(Vec4f::Dot3(xaxis, -eyePosition),
                           Vec4f::Dot3(yaxis, -eyePosition),
                           Vec4f::Dot3(zaxis, -eyePosition),
                           1.0f));
}

const Matrix4 Matrix4::MakePerspective(float aspect, float fovY, float nearZ, float farZ)
{
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;

    return Matrix4(Vec4f(xScale,  0.0f,   0.0f,                           0.0f),
                   Vec4f(0.0f,    yScale, 0.0f,                           0.0f),
                   Vec4f(0.0f,    0.0f,   farZ / (farZ - nearZ),          1.0f),
                   Vec4f(0.0f,    0.0f,   -nearZ * farZ / (farZ - nearZ), 0.0f));
}

const Matrix4 Matrix4::MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    return Matrix4(
               Vec4f(2.0f / (right - left), 0.0f,                  0.0f,                  0.0f),
               Vec4f(0.0f,                  2.0f / (top - bottom), 0.0f,                  0.0f),
               Vec4f(0.0f,                  0.0f,                  1.0f / (zFar - zNear), 0.0f),
               Vec4f((left + right) / (left - right),
                       (top + bottom) / (bottom - top),
                       zNear / (zNear - zFar),
                       1.0f));
}

const Matrix4 Matrix4::MakeScaling(const Vec4f& scale)
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

    Vec8f a0, a1, b0, b1;
    Vec8f c0, c1;

    b0 = _mm256_permute2f128_ps(b.vec[0], b.vec[0], 0x00);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(0, 0, 0, 0));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(0, 0, 0, 0));
    c0 = a0 * b0;
    c1 = a1 * b0;

    b0 = _mm256_permute2f128_ps(b.vec[0], b.vec[0], 0x11);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(1, 1, 1, 1));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(1, 1, 1, 1));
    c0 = Vec8f::MulAndAdd(a0, b0, c0);
    c1 = Vec8f::MulAndAdd(a1, b0, c1);

    b1 = _mm256_permute2f128_ps(b.vec[1], b.vec[1], 0x00);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(2, 2, 2, 2));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(2, 2, 2, 2));
    c0 = Vec8f::MulAndAdd(a0, b1, c0);
    c1 = Vec8f::MulAndAdd(a1, b1, c1);

    b1 = _mm256_permute2f128_ps(b.vec[1], b.vec[1], 0x11);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(3, 3, 3, 3));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(3, 3, 3, 3));
    c0 = Vec8f::MulAndAdd(a0, b1, c0);
    c1 = Vec8f::MulAndAdd(a1, b1, c1);

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
    Vec4f v0 = rows[2].Swizzle<1, 0, 0, 0>();
    Vec4f v1 = rows[3].Swizzle<2, 2, 1, 1>();
    Vec4f v2 = rows[2].Swizzle<1, 0, 0, 0>();
    Vec4f v3 = rows[3].Swizzle<3, 3, 3, 2>();
    Vec4f v4 = rows[2].Swizzle<2, 2, 1, 1>();
    Vec4f v5 = rows[3].Swizzle<3, 3, 3, 2>();

    Vec4f p0 = v0 * v1;
    Vec4f p1 = v2 * v3;
    Vec4f p2 = v4 * v5;

    v0 = rows[2].Swizzle<2, 2, 1, 1>();
    v1 = rows[3].Swizzle<1, 0, 0, 0>();
    v2 = rows[2].Swizzle<3, 3, 3, 2>();
    v3 = rows[3].Swizzle<1, 0, 0, 0>();
    v4 = rows[2].Swizzle<3, 3, 3, 2>();
    v5 = rows[3].Swizzle<2, 2, 1, 1>();

    p0 = Vec4f::NegMulAndAdd(v0, v1, p0);
    p1 = Vec4f::NegMulAndAdd(v2, v3, p1);
    p2 = Vec4f::NegMulAndAdd(v4, v5, p2);

    v0 = rows[1].Swizzle<3, 3, 3, 2>();
    v1 = rows[1].Swizzle<2, 2, 1, 1>();
    v2 = rows[1].Swizzle<1, 0, 0, 0>();

    Vec4f r = v0 * p0;
    r = Vec4f::NegMulAndAdd(v1, p1, r);
    r = Vec4f::MulAndAdd(v2, p2, r);

    Vec4f s = rows[0] * Vec4f{ 1.0f, -1.0f, 1.0f, -1.0f };
    return Vec4f::Dot4(s, r);
}

// for row major matrix
// we use Vec4f to represent 2x2 matrix as A = | A0  A1 |
//                                               | A2  A3 |
// 2x2 row major Matrix multiply A*B
NFE_FORCE_INLINE static const Vec4f Mat2Mul(const Vec4f vec1, const Vec4f vec2)
{
    return vec1 * vec2.Swizzle<0, 3, 0, 3>() + vec1.Swizzle<1, 0, 3, 2>() * vec2.Swizzle<2, 1, 2, 1>();
}
// 2x2 row major Matrix adjugate multiply (A#)*B
NFE_FORCE_INLINE static const Vec4f Mat2AdjMul(const Vec4f vec1, const Vec4f vec2)
{
    return vec1.Swizzle<3, 3, 0, 0>() * vec2 - vec1.Swizzle<1, 1, 2, 2>() * vec2.Swizzle<2, 3, 0, 1>();

}
// 2x2 row major Matrix multiply adjugate A*(B#)
NFE_FORCE_INLINE static const Vec4f Mat2MulAdj(const Vec4f vec1, const Vec4f vec2)
{
    return vec1 * vec2.Swizzle<3, 0, 3, 0>() - vec1.Swizzle<1, 0, 3, 2>() * vec2.Swizzle<2, 1, 2, 1>();
}

const Matrix4 Matrix4::Inverted() const
{
    // based on:
    // https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

    const Vec4f A = Vec4f::Shuffle<0,1,0,1>(rows[0], rows[1]);
    const Vec4f B = Vec4f::Shuffle<2,3,2,3>(rows[0], rows[1]);
    const Vec4f C = Vec4f::Shuffle<0,1,0,1>(rows[2], rows[3]);
    const Vec4f D = Vec4f::Shuffle<2,3,2,3>(rows[2], rows[3]);

    // determinant as (|A| |B| |C| |D|)
    const Vec4f detSub =
        Vec4f::Shuffle<0,2,0,2>(rows[0], rows[2]) * Vec4f::Shuffle<1,3,1,3>(rows[1], rows[3]) -
        Vec4f::Shuffle<1,3,1,3>(rows[0], rows[2]) * Vec4f::Shuffle<0,2,0,2>(rows[1], rows[3]);

    const Vec4f detA = detSub.SplatX();
    const Vec4f detB = detSub.SplatY();
    const Vec4f detC = detSub.SplatZ();
    const Vec4f detD = detSub.SplatW();

    const Vec4f D_C = Mat2AdjMul(D, C); // D#C
    const Vec4f A_B = Mat2AdjMul(A, B); // A#B

    // tr((A#B)(D#C))
    const Vec4f tr = (A_B * D_C.Swizzle<0, 2, 1, 3>()).HorizontalSum();

    // |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
    const Vec4f detM = detA * detD + detB * detC - tr;

    // (1/|M|, -1/|M|, -1/|M|, 1/|M|)
    const Vec4f rDetM = Vec4f(1.0f, -1.0f, -1.0f, 1.0f) / detM;

    Vec4f X = rDetM * Vec4f::MulAndSub(detD, A, Mat2Mul(B, D_C));       // X# = |D|A - B(D#C)
    Vec4f W = rDetM * Vec4f::MulAndSub(detA, D, Mat2Mul(C, A_B));       // W# = |A|D - C(A#B)
    Vec4f Y = rDetM * Vec4f::MulAndSub(detB, C, Mat2MulAdj(D, A_B));    // Y# = |B|C - D(A#B)#
    Vec4f Z = rDetM * Vec4f::MulAndSub(detC, B, Mat2MulAdj(A, D_C));    // Z# = |C|B - A(D#C)#

    Matrix4 r;

    // apply adjugate and store, here we combine adjugate shuffle and store shuffle
    r.rows[0] = Vec4f::Shuffle<3,1,3,1>(X, Y);
    r.rows[1] = Vec4f::Shuffle<2,0,2,0>(X, Y);
    r.rows[2] = Vec4f::Shuffle<3,1,3,1>(Z, W);
    r.rows[3] = Vec4f::Shuffle<2,0,2,0>(Z, W);

    return r;
}

const Box Matrix4::TransformBox(const Box& box) const
{
    // based on:
    // http://dev.theomader.com/transform-bounding-boxes/

    const Vec4f xa = rows[0] * box.min.x;
    const Vec4f xb = rows[0] * box.max.x;
    const Vec4f ya = rows[1] * box.min.y;
    const Vec4f yb = rows[1] * box.max.y;
    const Vec4f za = rows[2] * box.min.z;
    const Vec4f zb = rows[2] * box.max.z;

    return Box(
        Vec4f::Min(xa, xb) + Vec4f::Min(ya, yb) + Vec4f::Min(za, zb) + rows[3],
        Vec4f::Max(xa, xb) + Vec4f::Max(ya, yb) + Vec4f::Max(za, zb) + rows[3]
    );
}

} // namespace Math
} // namespace NFE
