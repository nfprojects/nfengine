struct VertexShaderOutput
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Pos : SV_POSITION;
};

struct PixelShaderOutput
{
    float4 color0 : SV_TARGET0;
#if (TARGETS > 1)
    float4 color1 : SV_TARGET1;
#endif
};

PixelShaderOutput main(VertexShaderOutput In)
{
    PixelShaderOutput Out;
    Out.color0 = In.Color;
#if (TARGETS > 1)
    Out.color1 = float4(1.0f, 1.0f, 1.0f, 1.0f) - In.Color;
#endif
    return Out;
}
