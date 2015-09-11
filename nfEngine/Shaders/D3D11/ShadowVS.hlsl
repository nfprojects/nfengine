#include "ShadowCommon.hlsl"

cbuffer Global : register(b0)
{
    float4x4 gViewProjMatrix;
};

struct VertexShaderInput
{
    /// per-vertex data
    float3 pos : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;

    /// per-instance data (transposed 4x3 matrix)
    float4 wordMat0 : TEXCOORD1;
    float4 wordMat1 : TEXCOORD2;
    float4 wordMat2 : TEXCOORD3;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    // decode world matrix
    float4x4 worldMatrix = transpose(float4x4(input.wordMat0,
                                              input.wordMat1,
                                              input.wordMat2,
                                              float4(0, 0, 0, 1)));

    float4 worldPos = mul(float4(input.pos, 1), worldMatrix);
    output.pos = mul(worldPos, gViewProjMatrix);
    output.screenPos = output.pos;
    output.texCoord = input.texCoord;

    return output;
}