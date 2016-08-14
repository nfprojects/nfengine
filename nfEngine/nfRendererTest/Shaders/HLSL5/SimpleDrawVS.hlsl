#ifndef USE_CBUFFER
#define USE_CBUFFER 0
#endif

#if (USE_CBUFFER > 0)
cbuffer TestCBuffer : register(c0)
{
    float4 CBufferColor;
};
#endif

struct VertexShaderInput
{
    float3 Pos : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

struct VertexShaderOutput
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Pos : SV_POSITION;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.Pos = float4(input.Pos, 1.0f);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;

#if (USE_CBUFFER > 0)
    output.Color *= CBufferColor;
#endif

    return output;
}
