struct VertexShaderInput
{
    float3 Pos : POSITION;
};

struct VertexShaderOutput
{
    float4 Pos : SV_POSITION;
};

VertexShaderOutput main(VertexShaderInput In)
{
    VertexShaderOutput Out = (VertexShaderOutput)0;
    Out.Pos = float4(In.Pos, 1.0f);
    return Out;
}