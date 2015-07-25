cbuffer Global : register(b0)
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4 gViewportResInv;
    float4 gScreenScale;
};

cbuffer OmniLightProps : register(b1)
{
    float4 position;
    float4 radius;
    float4 color;
};

struct VertexShaderInput
{
    float3 pos : POSITION;
};

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float4 viewPos : POSITION;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output = (VertexShaderOutput)0;
    float4 worldPos = float4(position.xyz + input.pos * radius.x, 1.0f);
    output.viewPos = mul(worldPos, gViewMatrix);
    output.pos = mul(output.viewPos, gProjMatrix);
    return output;
}