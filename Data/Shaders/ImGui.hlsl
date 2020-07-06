struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : TEXCOORD1;
};

///////////////////////////////////////////////////////////////////////////////

#ifdef NFE_VERTEX_SHADER

cbuffer VertexCBuffer : register(b0)
{
    float4x4 gProjectionMatrix;
};

struct VertexShaderInput
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : TEXCOORD1;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.pos = mul(float4(input.pos.xy, 0.0f, 1.0f), gProjectionMatrix);
    output.col = input.col;
    output.uv = input.uv;
    return output;
}

#endif // NFE_VERTEX_SHADER

///////////////////////////////////////////////////////////////////////////////

#ifdef NFE_PIXEL_SHADER

sampler gSampler : register(s0);
Texture2D gTexture : register(t0);

float4 main(VertexShaderOutput input) : SV_Target
{
    return input.col * gTexture.Sample(gSampler, input.uv);
}

#endif // NFE_PIXEL_SHADER
