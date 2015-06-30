cbuffer Global : register(b0)
{
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
}

struct VertexShaderInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

struct VertexShaderOutput
{
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
    float4 pos : SV_POSITION;
};

//---------------------------------------------------------------

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output = (VertexShaderOutput)0;
    output.pos = mul(float4(input.pos, 1.0f), gViewMatrix);
    output.pos = mul(output.pos, gProjMatrix);
    output.color = input.color;
    output.texCoord = input.texCoord;
    return output;
}