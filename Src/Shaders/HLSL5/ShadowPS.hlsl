#include "ShadowCommon.hlsl"

cbuffer Global : register(b0)
{
    float4x4 gViewProjMatrix;
    float4 gLightPos;
};

float main(VertexShaderOutput input) : SV_TARGET0
{
    // TODO: alpha testing

#if (CUBE_SHADOW_MAP > 0)
    float depth = length(input.worldPos - gLightPos.xyz);
#else
    float depth = input.screenPos.z / input.screenPos.w;
#endif

    // TODO: make it adjustable
    const float depthBias = 0.0001f;
    const float derivativeFactor = 2.0f;

    float dx = ddx(depth);
    float dy = ddy(depth);
    float depthDerivative = sqrt(dx*dx + dy*dy);

    return depth + depthBias + depthDerivative * derivativeFactor;
}