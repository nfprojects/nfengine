#ifndef USE_TEXTURE
#define USE_TEXTURE 0
#endif

#if (USE_TEXTURE > 0)
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
#endif

struct VertexShaderOutput
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

float4 main(VertexShaderOutput input) : SV_TARGET0
{
#if (USE_TEXTURE > 0)
    return input.Color * gTexture.Sample(gSampler, In.TexCoord);
#else
    return input.Color;
#endif
}
