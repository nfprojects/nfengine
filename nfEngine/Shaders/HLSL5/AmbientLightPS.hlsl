#include "LightCommon.hlsl"

cbuffer AmbientLightParams : register (b1)
{
    float4 gAmbientLight;
    float4 gBackgroundColor;
};

struct QuadVertexShaderOutput
{
    float4 pos : SV_POSITION;
};

float4 main(QuadVertexShaderOutput input) : SV_TARGET0
{
    int3 texelCoords = int3((int2)input.pos.xy, 0);
    float depth = gDepthTex.Load(texelCoords);

    float4 result = gBackgroundColor;
    if (depth < gInfinityDist)
    {
        float4 diffuseColor = gGBufferTex1.Load(texelCoords);
        result = diffuseColor * gAmbientLight;
        result += gGBufferTex2.Load(texelCoords); // emission color
    }

    return float4(result.xyz, 1.0f);
}