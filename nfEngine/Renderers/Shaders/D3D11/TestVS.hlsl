struct VS_IN
{
    float3 Pos : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

struct VS_OUTPUT
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Pos : SV_POSITION;
};

VS_OUTPUT main(VS_IN In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    Out.Pos = float4(In.Pos, 1.0f);
    Out.TexCoord = In.TexCoord;
    Out.Color = In.Color;
    return Out;
}