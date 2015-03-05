struct VS_IN
{
    float3 Pos : POSITION;
    float4 Color : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Color : TEXCOORD;
    float4 Pos : SV_POSITION;
};

VS_OUTPUT main(VS_IN In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    Out.Pos = float4(In.Pos, 1.0f);
    Out.Color = In.Color;
    return Out;
}