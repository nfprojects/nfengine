struct VS_INPUT_OUTPUT
{
    float4 Rect      : POSITION;
    float4 TexCoords : TEXCOORD0;
    float4 Color     : TEXCOORD1;
};

struct GS_OUT
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
};