#if (USE_TEXTURE > 0)
Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);
#endif // (USE_TEXTURE > 0)

struct VertexShaderOutput
{
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD0;
    float4 pos      : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VertexShaderOutput input) : SV_TARGET0
{
    float4 color = input.color;

#if (USE_TEXTURE > 0)
    color *= gTexture.Sample(gSampler, input.texCoord);
#endif // (USE_TEXTURE > 0)

    return color;
}