#include "OmniLightCommon.hlsl"

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output = (VertexShaderOutput)0;
    float4 worldPos = float4(gLightPos.xyz + input.pos * gLightRadius.x, 1.0f);
    output.viewPos = mul(worldPos, gViewMatrix);
    output.pos = mul(output.viewPos, gProjMatrix);
    return output;
}