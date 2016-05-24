#include "SpotLightCommon.hlsl"

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    // vertex position transformation
    float4 worldPos = mul(float4(input.pos, 1.0f), gLightViewProjMatrixInv);
    output.viewPos = mul(worldPos, gViewMatrix);
    output.pos = mul(output.viewPos, gProjMatrix);

    return output;
}