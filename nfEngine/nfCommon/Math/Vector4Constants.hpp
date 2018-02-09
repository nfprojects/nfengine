/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Vector4 constants definitions.
 */

#pragma once

#include "Vector4.hpp"
#include <float.h>


namespace NFE {
namespace Math {


// used to initialize float constants
struct NFE_ALIGN(16) Vector4f
{
    union
    {
        float f[4];
#ifdef NFE_USE_SSE
        __m128 v;
#endif
    };

#ifdef NFE_USE_SSE
    operator Vector4() const
    {
        Vector4 temp;
        temp.v = v;
        return temp;
    }

    operator __m128() const
    {
        return v;
    }

    operator __m128i() const
    {
        return reinterpret_cast<const __m128i*>(&v)[0];
    }

    operator __m128d() const
    {
        return reinterpret_cast<const __m128d*>(&v)[0];
    }
#else
    operator Vector4() const
    {
        Vector4 temp;
        temp.f[0] = f[0];
        temp.f[1] = f[1];
        temp.f[2] = f[2];
        temp.f[3] = f[3];
        return temp;
    }
#endif // NFE_USE_SSE
};


// used to initialize integer constants
struct NFE_ALIGN(16) Vector4i
{
#ifdef NFE_USE_SSE
    union
    {
        unsigned int u[4];
        __m128 v;
    };

    operator Vector4() const
    {
        Vector4 temp;
        temp.v = v;
        return temp;
    }

    operator __m128() const
    {
        return v;
    }

    operator __m128i() const
    {
        return reinterpret_cast<const __m128i*>(&v)[0];
    }

    operator __m128d() const
    {
        return reinterpret_cast<const __m128d*>(&v)[0];
    }
#else
    union
    {
        unsigned int u[4];
    };

    operator Vector4() const
    {
        Vector4 temp;
        temp.u[0] = u[0];
        temp.u[1] = u[1];
        temp.u[2] = u[2];
        temp.u[3] = u[3];
        return temp;
    }
#endif
};


const Vector4f VECTOR_MAX = { { { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } } };
const Vector4f VECTOR_NEG_MAX = { { { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX } } };
const Vector4f VECTOR_MIN = { { { FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN } } };
const Vector4f VECTOR_NEG_MIN = { { { -FLT_MIN, -FLT_MIN, -FLT_MIN, -FLT_MIN } } };
const Vector4f VECTOR_EPSILON = { { { NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON, NFE_MATH_EPSILON } } };
const Vector4f VECTOR_ONE = { { { 1.0f, 1.0f, 1.0f, 1.0f } } };
const Vector4f VECTOR_ONE3 = { { { 1.0f, 1.0f, 1.0f, 0.0f } } };
const Vector4f VECTOR_MINUS_ONE = { { { -1.0f, -1.0f, -1.0f, -1.0f } } };
const Vector4i VECTOR_MASK_X = { { { 0xFFFFFFFF, 0, 0, 0 } } };
const Vector4i VECTOR_MASK_Y = { { { 0, 0xFFFFFFFF, 0, 0 } } };
const Vector4i VECTOR_MASK_Z = { { { 0, 0, 0xFFFFFFFF, 0 } } };
const Vector4i VECTOR_MASK_W = { { { 0, 0, 0, 0xFFFFFFFF } } };
const Vector4i VECTOR_MASK_XY = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0, 0 } } };
const Vector4i VECTOR_MASK_XYZ = { { { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 } } };
const Vector4i VECTOR_MASK_ABS = { { { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF } } };

const Vector4f VECTOR_INV_255 = { { { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f } } };
const Vector4f VECTOR_255 = { { { 255.0f, 255.0f, 255.0f, 255.0f } } };

/// identity matrix rows
const Vector4f VECTOR_X = { { { 1.0f, 0.0f, 0.0f, 0.0f } } };
const Vector4f VECTOR_Y = { { { 0.0f, 1.0f, 0.0f, 0.0f } } };
const Vector4f VECTOR_Z = { { { 0.0f, 0.0f, 1.0f, 0.0f } } };
const Vector4f VECTOR_W = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };


} // namespace Math
} // namespace NFE
