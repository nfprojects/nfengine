cbuffer Global : register(b0)
{
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
}

cbuffer PerMesh : register(b1)
{
    float4x4 gModelMatrix;
}

struct VertexShaderInput
{
#if (IS_MESH > 0)
    float3 pos      : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 normal   : TEXCOORD1;
    float4 tangent  : TEXCOORD2;
#else
    float3 pos      : POSITION;
    float4 color    : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
#endif // (IS_MESH > 0)
};

struct VertexShaderOutput
{
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD0;
    float4 pos      : SV_POSITION;
};

//---------------------------------------------------------------

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output = (VertexShaderOutput)0;

#if (IS_MESH > 0)
    output.color = float4(1.0, 1.0, 1.0, 1.0);
    output.pos = mul(float4(input.pos, 1.0f), gModelMatrix);
    output.pos = mul(output.pos, gViewMatrix);
#else
    output.color = input.color;
    output.pos = mul(float4(input.pos, 1.0f), gViewMatrix);
#endif // (IS_MESH > 0)

    output.pos = mul(output.pos, gProjMatrix);

    output.texCoord = input.texCoord;
    return output;
}