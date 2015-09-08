cbuffer Global : register(b0)
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4 gViewportResInv;
    float4 gScreenScale;
};

cbuffer SpotLightProps : register(b1)
{
    float4 gLightPos;
    float4 gDirection;
    float4 gLightColor;
    float4 gFarDist;
    row_major float4x4 gLightViewProjMatrix;
    row_major float4x4 gLightViewProjMatrixInv;
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
    VertexShaderOutput output;

    // vertex position transformation
    float4 worldPos = mul(float4(input.pos, 1.0f), gLightViewProjMatrixInv);
    output.viewPos = mul(worldPos, gViewMatrix);
    output.pos = mul(output.viewPos, gProjMatrix);

    return output;
}