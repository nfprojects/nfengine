sampler gSampler : register(s0);
Texture2D gTexture : register(t0);

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : TEXCOORD1;
};

float4 main(VertexShaderOutput input) : SV_Target
{
    return input.col * gTexture.Sample(gSampler, input.uv);
}