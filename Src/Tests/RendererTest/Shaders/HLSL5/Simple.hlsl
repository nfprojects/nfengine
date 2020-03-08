struct VS_IN
{
    float3 Pos : POSITION;
    float4 Color : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 Color : TEXCOORD0;
    float4 Pos : SV_POSITION;
};

cbuffer TestCBuffer : register(b1)
{
    float4x4 viewMatrix;
};

Texture2D<float4> TestTexture : register(t3);
SamplerState TestSampler : register(s3);

VS_OUTPUT main(VS_IN In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    Out.Pos = float4(In.Pos, 1.0f);
    Out.Pos = mul(Out.Pos, viewMatrix);
    Out.Color = In.Color * TestTexture.SampleLevel(TestSampler, In.TexCoord, 0);
    return Out;
}
