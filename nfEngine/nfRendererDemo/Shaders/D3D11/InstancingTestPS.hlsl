struct VertexShaderOutput
{
    float4 Pos : SV_POSITION;
    float4 Color : TEXCOORD0;
};

float4 main(VertexShaderOutput input) : SV_TARGET0
{
    return input.Color;
}
