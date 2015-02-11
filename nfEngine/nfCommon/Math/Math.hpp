/**
 * @file   Math.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic math classes and functions declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include <math.h>

#define NFE_MATH_USE_SSE

#ifdef NFE_MATH_USE_SSE
#include <xmmintrin.h>
#include <mmintrin.h>
#include <emmintrin.h>
//#include <smmintrin.h>
#endif


#define NFE_INLINE __forceinline
#ifdef WIN32
#define NFE_ALIGN(a) __declspec(align(a))
#else // WIN32
#define NFE_ALIGN(a) __attribute__((aligned(a)))
#endif // WIN32

#define NFE_MATH_EPSILON (0.000001f)
#define NFE_MATH_PI (3.14159265359f)
#define NFE_MATH_2PI (6.28318530718f)
#define NFE_MATH_E (2.7182818f)

namespace NFE {
namespace Math {

//
// structures for vector storing
//
struct Float2
{
    float x, y;

    Float2() : x(0.0f), y(0.0f) {};
    Float2(float x_, float y_) : x(x_), y(y_) {};
    Float2(float* pArr) : x(pArr[0]), y(pArr[1]) {};
};

struct Float3
{
    float x, y, z;

    Float3() : x(0.0f), y(0.0f), z(0.0f) {};
    Float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
    Float3(float* pArr) : x(pArr[0]), y(pArr[1]), z(pArr[2]) {};
};

struct Float4
{
    float x, y, z, w;

    Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {};
    Float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {};
    Float4(float* pArr) : x(pArr[0]), y(pArr[1]), z(pArr[2]), w(pArr[3]) {};
};


//predeclarations
class Vector;
class Matrix;
class Box;
class Frustum;
class Sphere;



union FloatInt
{
    float f;
    unsigned int u;
};

union DoubleInt
{
    double f;
    NFE::Common::uint64 u;
};

template<typename T>
NFE_INLINE T Min(const T a, const T b)
{
    if (a < b)
        return a;

    return b;
}

template<typename T>
NFE_INLINE T Max(const T a, const T b)
{
    if (a > b)
        return a;

    return b;
}

//returns x with sign of y
NFE_INLINE float CopySignF(const float x, const float y)
{
    FloatInt _x, _y;
    _x.f = x;
    _y.f = y;
    _x.u = (0x7fffffff & _x.u) | (0x80000000 & _y.u);
    return _x.f;
}

// linear interpolation
template<typename T>
NFE_INLINE T Lerp(const T a, const T b, const T w)
{
    return a + w * (b - a);
}

// Wang hash
NFE_INLINE unsigned int Hash(unsigned int x)
{
    x = (x ^ 61) ^ (x >> 16);
    x *= 9;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
}

} // namespace Math
} // namespace NFE


// TODO: remove
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Geometry.hpp"
