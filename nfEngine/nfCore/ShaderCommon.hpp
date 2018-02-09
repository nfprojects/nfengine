#pragma once

#include "nfCommon/Math/Vector4.hpp"
#include "nfCommon/Math/Matrix.hpp"


namespace NFE {

// translate HLSL types to C++ types
typedef Math::Vector4 float4;
typedef Math::Float3 float3;
typedef Math::Float2 float2;
typedef Math::Matrix float4x4;

struct Int4
{
    int i[4];
};

struct Int3
{
    int i[3];
};

struct Int2
{
    int i[2];
};

struct Uint4
{
    unsigned int i[4];
};

struct Uint3
{
    unsigned int i[3];
};

struct Uint2
{
    unsigned int i[2];
};

typedef Int4 int4;
typedef Int3 int3;
typedef Int2 int2;
typedef Uint4 uint4;
typedef Uint3 uint3;
typedef Uint2 uint2;

} // namespace NFE