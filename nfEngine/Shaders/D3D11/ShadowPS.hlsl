#include "ShadowCommon.hlsl"

cbuffer Global : register(b0)
{
    float4x4 gViewProjMatrix;
};

float main(VertexShaderOutput input) : SV_TARGET0
{
    // TODO: alpha testing

    const float depthBias = 0.0001f; // TODO: make it adjustable
    float Depth = input.screenPos.z / input.screenPos.w;
    return Depth + depthBias;
}