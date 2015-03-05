struct VS_OUTPUT
{
    float4 Color : TEXCOORD;
    float4 Pos : SV_POSITION;
};

float4 main(VS_OUTPUT In) : SV_TARGET0
{
    return In.Color;
}