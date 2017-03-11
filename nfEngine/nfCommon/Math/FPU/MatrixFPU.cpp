/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU-specific Matrix function definitions.
 */

#include "PCH.hpp"
#include "../Matrix.hpp"


#ifndef NFE_USE_SSE

namespace NFE {
namespace Math {

Matrix MatrixRotationNormal(const Vector& normalAxis, float angle)
{
    Matrix result;

    float x = normalAxis[0];
    float y = normalAxis[1];
    float z = normalAxis[2];
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    result.r[0] = Vector(t * x * x + c,
                         t * x * y - s * z,
                         t * x *z + s * y,
                         0.0f);

    result.r[1] = Vector(t * x * y + s * z,
                         t * y * y + c,
                         t * y * z - s * x,
                         0.0f);

    result.r[2] = Vector(t * x * z - s * y,
                         t * y * z + s * x,
                         t * z * z + c,
                         0.0f);

    result.r[3] = VECTOR_IDENTITY_ROW_3;

    return result;
}

Matrix MatrixInverse(const Matrix& m)
{
    float inv[16], det;

    // formula taken from MESA implmentation of GLU library
    inv[0 ] =  m.f[5 ] * m.f[10] * m.f[15] - m.f[5 ] * m.f[11] * m.f[14] -
               m.f[9 ] * m.f[6 ] * m.f[15] + m.f[9 ] * m.f[7 ] * m.f[14] +
               m.f[13] * m.f[6 ] * m.f[11] - m.f[13] * m.f[7 ] * m.f[10];
    inv[1 ] = -m.f[1 ] * m.f[10] * m.f[15] + m.f[1 ] * m.f[11] * m.f[14] +
               m.f[9 ] * m.f[2 ] * m.f[15] - m.f[9 ] * m.f[3 ] * m.f[14] -
               m.f[13] * m.f[2 ] * m.f[11] + m.f[13] * m.f[3 ] * m.f[10];
    inv[2 ] =  m.f[1 ] * m.f[6 ] * m.f[15] - m.f[1 ] * m.f[7 ] * m.f[14] -
               m.f[5 ] * m.f[2 ] * m.f[15] + m.f[5 ] * m.f[3 ] * m.f[14] +
               m.f[13] * m.f[2 ] * m.f[7 ] - m.f[13] * m.f[3 ] * m.f[6 ];
    inv[3 ] = -m.f[1 ] * m.f[6 ] * m.f[11] + m.f[1 ] * m.f[7 ] * m.f[10] +
               m.f[5 ] * m.f[2 ] * m.f[11] - m.f[5 ] * m.f[3 ] * m.f[10] -
               m.f[9 ] * m.f[2 ] * m.f[7 ] + m.f[9 ] * m.f[3 ] * m.f[6 ];
    inv[4 ] = -m.f[4 ] * m.f[10] * m.f[15] + m.f[4 ] * m.f[11] * m.f[14] +
               m.f[8 ] * m.f[6 ] * m.f[15] - m.f[8 ] * m.f[7 ] * m.f[14] -
               m.f[12] * m.f[6 ] * m.f[11] + m.f[12] * m.f[7 ] * m.f[10];
    inv[5 ] =  m.f[0 ] * m.f[10] * m.f[15] - m.f[0 ] * m.f[11] * m.f[14] -
               m.f[8 ] * m.f[2 ] * m.f[15] + m.f[8 ] * m.f[3 ] * m.f[14] +
               m.f[12] * m.f[2 ] * m.f[11] - m.f[12] * m.f[3 ] * m.f[10];
    inv[6 ] = -m.f[0 ] * m.f[6 ] * m.f[15] + m.f[0 ] * m.f[7 ] * m.f[14] +
               m.f[4 ] * m.f[2 ] * m.f[15] - m.f[4 ] * m.f[3 ] * m.f[14] -
               m.f[12] * m.f[2 ] * m.f[7 ] + m.f[12] * m.f[3 ] * m.f[6 ];
    inv[7 ] =  m.f[0 ] * m.f[6 ] * m.f[11] - m.f[0 ] * m.f[7 ] * m.f[10] -
               m.f[4 ] * m.f[2 ] * m.f[11] + m.f[4 ] * m.f[3 ] * m.f[10] +
               m.f[8 ] * m.f[2 ] * m.f[7 ] - m.f[8 ] * m.f[3 ] * m.f[6 ];
    inv[8 ] =  m.f[4 ] * m.f[9 ] * m.f[15] - m.f[4 ] * m.f[11] * m.f[13] -
               m.f[8 ] * m.f[5 ] * m.f[15] + m.f[8 ] * m.f[7 ] * m.f[13] +
               m.f[12] * m.f[5 ] * m.f[11] - m.f[12] * m.f[7 ] * m.f[9 ];
    inv[9 ] = -m.f[0 ] * m.f[9 ] * m.f[15] + m.f[0 ] * m.f[11] * m.f[13] +
               m.f[8 ] * m.f[1 ] * m.f[15] - m.f[8 ] * m.f[3 ] * m.f[13] -
               m.f[12] * m.f[1 ] * m.f[11] + m.f[12] * m.f[3 ] * m.f[9 ];
    inv[10] =  m.f[0 ] * m.f[5 ] * m.f[15] - m.f[0 ] * m.f[7 ] * m.f[13] -
               m.f[4 ] * m.f[1 ] * m.f[15] + m.f[4 ] * m.f[3 ] * m.f[13] +
               m.f[12] * m.f[1 ] * m.f[7 ] - m.f[12] * m.f[3 ] * m.f[5 ];
    inv[11] = -m.f[0 ] * m.f[5 ] * m.f[11] + m.f[0 ] * m.f[7 ] * m.f[9 ] +
               m.f[4 ] * m.f[1 ] * m.f[11] - m.f[4 ] * m.f[3 ] * m.f[9 ] -
               m.f[8 ] * m.f[1 ] * m.f[7 ] + m.f[8 ] * m.f[3 ] * m.f[5 ];
    inv[12] = -m.f[4 ] * m.f[9 ] * m.f[14] + m.f[4 ] * m.f[10] * m.f[13] +
               m.f[8 ] * m.f[5 ] * m.f[14] - m.f[8 ] * m.f[6 ] * m.f[13] -
               m.f[12] * m.f[5 ] * m.f[10] + m.f[12] * m.f[6 ] * m.f[9 ];
    inv[13] =  m.f[0 ] * m.f[9 ] * m.f[14] - m.f[0 ] * m.f[10] * m.f[13] -
               m.f[8 ] * m.f[1 ] * m.f[14] + m.f[8 ] * m.f[2 ] * m.f[13] +
               m.f[12] * m.f[1 ] * m.f[10] - m.f[12] * m.f[2 ] * m.f[9 ];
    inv[14] = -m.f[0 ] * m.f[5 ] * m.f[14] + m.f[0 ] * m.f[6 ] * m.f[13] +
               m.f[4 ] * m.f[1 ] * m.f[14] - m.f[4 ] * m.f[2 ] * m.f[13] -
               m.f[12] * m.f[1 ] * m.f[6 ] + m.f[12] * m.f[2 ] * m.f[5 ];
    inv[15] =  m.f[0 ] * m.f[5 ] * m.f[10] - m.f[0 ] * m.f[6 ] * m.f[9 ] -
               m.f[4 ] * m.f[1 ] * m.f[10] + m.f[4 ] * m.f[2 ] * m.f[9 ] +
               m.f[8 ] * m.f[1 ] * m.f[6 ] - m.f[8 ] * m.f[2 ] * m.f[5 ];

    det = m.f[0] * inv[0] + m.f[1] * inv[4] + m.f[2] * inv[8] + m.f[3] * inv[12];

    return Matrix({inv[0],  inv[1],  inv[2],  inv[3],
                   inv[4],  inv[5],  inv[6],  inv[7],
                   inv[8],  inv[9],  inv[10], inv[11],
                   inv[12], inv[13], inv[14], inv[15]}) / det;
}

} // namespace Math
} // namespace NFE

#endif // NFE_USE_SSE