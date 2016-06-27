#include "Common.hlsl"
#include "ShadowCommon.hlsl"

cbuffer Global : register(b0)
{
    float4x4 gViewProjMatrix;
    float4 gLightPos;
};

VertexShaderOutput main(MeshVertexShaderInput input)
{
    VertexShaderOutput output;

    // decode world matrix
    float4x4 worldMatrix = transpose(float4x4(input.worldMat0,
                                              input.worldMat1,
                                              input.worldMat2,
                                              float4(0.0f, 0.0f, 0.0f, 1.0f)));

    float4 worldPos = mul(float4(input.pos, 1.0f), worldMatrix);
    output.pos = mul(worldPos, gViewProjMatrix);
    output.screenPos = output.pos;
    output.worldPos = worldPos.xyz;
    output.texCoord = input.texCoord;

    return output;
}