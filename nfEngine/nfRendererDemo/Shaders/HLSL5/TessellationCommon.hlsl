struct VertexShaderOutput
{
    float3 pos : POSITION;
};

struct HullShaderConstantOutput
{
    float edges[2] : SV_TessFactor;
};

struct HullShaderOutput
{
    float3 pos : POSITION;
};

struct DomainShaderOutput
{
    float4 position : SV_Position;
};