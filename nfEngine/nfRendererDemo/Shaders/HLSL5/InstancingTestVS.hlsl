struct VertexShaderInput
{
    float3 Pos : POSITION;
    float4 Color : TEXCOORD0;

#if (USE_INSTANCING > 0)
    float3 InstancePos : TEXCOORD1;
    float4 InstanceColor : TEXCOORD2;
#endif  // (USE_INSTANCING > 0)
};

struct VertexShaderOutput
{
    float4 Pos : SV_POSITION;
    float4 Color : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.Pos = float4(input.Pos, 1.0f);
    output.Color = input.Color;

#if (USE_INSTANCING > 0)
    output.Pos += float4(input.InstancePos, 0.0f);
    output.Color *= input.InstanceColor;
#endif // (USE_INSTANCING > 0)

    return output;
}
