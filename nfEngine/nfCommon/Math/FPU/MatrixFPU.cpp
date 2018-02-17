/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU-specific Matrix4 function definitions.
 */

#include "PCH.hpp"
#include "../Matrix4.hpp"


#ifndef NFE_USE_SSE

namespace NFE {
namespace Math {

Matrix4 Matrix4::MakeRotationNormal(const Vector4& normalAxis, float angle)
{
    Matrix4 result;

    float x = normalAxis[0];
    float y = normalAxis[1];
    float z = normalAxis[2];
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    result.r[0] = Vector4(t * x * x + c,
                         t * x * y - s * z,
                         t * x *z + s * y,
                         0.0f);

    result.r[1] = Vector4(t * x * y + s * z,
                         t * y * y + c,
                         t * y * z - s * x,
                         0.0f);

    result.r[2] = Vector4(t * x * z - s * y,
                         t * y * z + s * x,
                         t * z * z + c,
                         0.0f);

    result.r[3] = VECTOR_W;

    return result;
}

Matrix4 Matrix4::Inverted() const
{
    float inv[16], det;

    // formula taken from MESA implementation of GLU library
    inv[0 ] =  f[5 ] * f[10] * f[15] - f[5 ] * f[11] * f[14] -
               f[9 ] * f[6 ] * f[15] + f[9 ] * f[7 ] * f[14] +
               f[13] * f[6 ] * f[11] - f[13] * f[7 ] * f[10];
    inv[1 ] = -f[1 ] * f[10] * f[15] + f[1 ] * f[11] * f[14] +
               f[9 ] * f[2 ] * f[15] - f[9 ] * f[3 ] * f[14] -
               f[13] * f[2 ] * f[11] + f[13] * f[3 ] * f[10];
    inv[2 ] =  f[1 ] * f[6 ] * f[15] - f[1 ] * f[7 ] * f[14] -
               f[5 ] * f[2 ] * f[15] + f[5 ] * f[3 ] * f[14] +
               f[13] * f[2 ] * f[7 ] - f[13] * f[3 ] * f[6 ];
    inv[3 ] = -f[1 ] * f[6 ] * f[11] + f[1 ] * f[7 ] * f[10] +
               f[5 ] * f[2 ] * f[11] - f[5 ] * f[3 ] * f[10] -
               f[9 ] * f[2 ] * f[7 ] + f[9 ] * f[3 ] * f[6 ];
    inv[4 ] = -f[4 ] * f[10] * f[15] + f[4 ] * f[11] * f[14] +
               f[8 ] * f[6 ] * f[15] - f[8 ] * f[7 ] * f[14] -
               f[12] * f[6 ] * f[11] + f[12] * f[7 ] * f[10];
    inv[5 ] =  f[0 ] * f[10] * f[15] - f[0 ] * f[11] * f[14] -
               f[8 ] * f[2 ] * f[15] + f[8 ] * f[3 ] * f[14] +
               f[12] * f[2 ] * f[11] - f[12] * f[3 ] * f[10];
    inv[6 ] = -f[0 ] * f[6 ] * f[15] + f[0 ] * f[7 ] * f[14] +
               f[4 ] * f[2 ] * f[15] - f[4 ] * f[3 ] * f[14] -
               f[12] * f[2 ] * f[7 ] + f[12] * f[3 ] * f[6 ];
    inv[7 ] =  f[0 ] * f[6 ] * f[11] - f[0 ] * f[7 ] * f[10] -
               f[4 ] * f[2 ] * f[11] + f[4 ] * f[3 ] * f[10] +
               f[8 ] * f[2 ] * f[7 ] - f[8 ] * f[3 ] * f[6 ];
    inv[8 ] =  f[4 ] * f[9 ] * f[15] - f[4 ] * f[11] * f[13] -
               f[8 ] * f[5 ] * f[15] + f[8 ] * f[7 ] * f[13] +
               f[12] * f[5 ] * f[11] - f[12] * f[7 ] * f[9 ];
    inv[9 ] = -f[0 ] * f[9 ] * f[15] + f[0 ] * f[11] * f[13] +
               f[8 ] * f[1 ] * f[15] - f[8 ] * f[3 ] * f[13] -
               f[12] * f[1 ] * f[11] + f[12] * f[3 ] * f[9 ];
    inv[10] =  f[0 ] * f[5 ] * f[15] - f[0 ] * f[7 ] * f[13] -
               f[4 ] * f[1 ] * f[15] + f[4 ] * f[3 ] * f[13] +
               f[12] * f[1 ] * f[7 ] - f[12] * f[3 ] * f[5 ];
    inv[11] = -f[0 ] * f[5 ] * f[11] + f[0 ] * f[7 ] * f[9 ] +
               f[4 ] * f[1 ] * f[11] - f[4 ] * f[3 ] * f[9 ] -
               f[8 ] * f[1 ] * f[7 ] + f[8 ] * f[3 ] * f[5 ];
    inv[12] = -f[4 ] * f[9 ] * f[14] + f[4 ] * f[10] * f[13] +
               f[8 ] * f[5 ] * f[14] - f[8 ] * f[6 ] * f[13] -
               f[12] * f[5 ] * f[10] + f[12] * f[6 ] * f[9 ];
    inv[13] =  f[0 ] * f[9 ] * f[14] - f[0 ] * f[10] * f[13] -
               f[8 ] * f[1 ] * f[14] + f[8 ] * f[2 ] * f[13] +
               f[12] * f[1 ] * f[10] - f[12] * f[2 ] * f[9 ];
    inv[14] = -f[0 ] * f[5 ] * f[14] + f[0 ] * f[6 ] * f[13] +
               f[4 ] * f[1 ] * f[14] - f[4 ] * f[2 ] * f[13] -
               f[12] * f[1 ] * f[6 ] + f[12] * f[2 ] * f[5 ];
    inv[15] =  f[0 ] * f[5 ] * f[10] - f[0 ] * f[6 ] * f[9 ] -
               f[4 ] * f[1 ] * f[10] + f[4 ] * f[2 ] * f[9 ] +
               f[8 ] * f[1 ] * f[6 ] - f[8 ] * f[2 ] * f[5 ];

    det = f[0] * inv[0] + f[1] * inv[4] + f[2] * inv[8] + f[3] * inv[12];

    return Matrix4({inv[0],  inv[1],  inv[2],  inv[3],
                   inv[4],  inv[5],  inv[6],  inv[7],
                   inv[8],  inv[9],  inv[10], inv[11],
                   inv[12], inv[13], inv[14], inv[15]}) / det;
}

} // namespace Math
} // namespace NFE

#endif // NFE_USE_SSE