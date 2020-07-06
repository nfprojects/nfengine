struct VertexShaderOutput
{
    float4 pos      : SV_POSITION;
    float2 normal   : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

///////////////////////////////////////////////////////////////////////////////

#ifdef NFE_VERTEX_SHADER

cbuffer VertexCBuffer : register(b0)
{
    float4x4 gProjectionMatrix;
};

struct VertexShaderInput
{
    float3 pos      : POSITION;
    float3 normal   : TEXCOORD0;
    float2 uv       : TEXCOORD1;
    float4 col      : TEXCOORD2;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.pos = mul(float4(input.pos.xy, 0.0f, 1.0f), gProjectionMatrix);
    output.normal = input.normal; // TODO transform
    output.uv = input.uv;
    return output;
}

#endif // NFE_VERTEX_SHADER

///////////////////////////////////////////////////////////////////////////////

#ifdef NFE_PIXEL_SHADER

struct PixelShaderOutput
{
    float4 gbuffer0 : SV_Target0;
    float4 gbuffer1 : SV_Target1;
    float2 gbuffer2 : SV_Target2;
};

//sampler gSampler : register(s0);
//Texture2D gTexture : register(t0);

PixelShaderInput main(VertexShaderOutput input) : SV_Target
{
    PixelShaderOutput output;
    output.gbuffer0 = float4(0.5f, 0.5f, 0.5f, 1.0f);
    output.gbuffer1 = float4(0.0f, 1.0f, 0.0f, 1.0f);
    output.gbuffer2 = float4(0.5f, 0.5f);
    return output;
}

#endif // NFE_PIXEL_SHADER
