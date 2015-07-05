#if (USE_TEXTURE > 0)
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
#endif

struct VS_OUTPUT
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Pos : SV_POSITION;
};

float4 main(VS_OUTPUT In) : SV_TARGET0
{
#if (USE_TEXTURE > 0)
    return In.Color * gTexture.Sample(gSampler, In.TexCoord);
#else
    return In.Color;
#endif
}
