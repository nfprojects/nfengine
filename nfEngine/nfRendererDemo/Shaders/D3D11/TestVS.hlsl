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

#if (USE_CBUFFER > 0)
cbuffer TestCBuffer : register(c0)
{
    float4x4 viewMatrix;
};
#endif

float4x4 defaultViewMatrix = { 1.0, 0.0, 0.0, 0.0,
                               0.0, 1.0, 0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 1.0 };

VertexShaderOutput main(VertexShaderInput In)
{
    VertexShaderOutput Out = (VertexShaderOutput)0;
    Out.Pos = float4(In.Pos, 1.0f);

#if (USE_CBUFFER > 0)
    Out.Pos = mul(Out.Pos, viewMatrix);
#else
    Out.Pos = mul(Out.Pos, defaultViewMatrix);
#endif

    Out.TexCoord = In.TexCoord;
    Out.Color = In.Color;
    return Out;
}
