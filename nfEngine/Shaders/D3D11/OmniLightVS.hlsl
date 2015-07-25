cbuffer Global : register(b0)
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4x4 gProjInverseMatrix;
    float4x4 gViewProjMatrix;
    float4x4 gViewProjInverseMatrix;
    float4 gViewportResInv;
    float4 gScreenScale;
};

cbuffer OmniLightProps : register(b1)
{
    float4 Position;
    float4 Radius;
    float4 Color;
};

struct VertexShaderInput
{
    float3 Pos : POSITION;
};

struct VertexShaderOutput
{
    float4 Pos : SV_POSITION;
    float4 ViewPos : POSITION;
};

//---------------------------------------------------------------

VertexShaderOutput main(VertexShaderInput In)
{
    VertexShaderOutput Out = (VertexShaderOutput)0;
    float4 WorldPos = float4(Position.xyz + In.Pos * Radius.x, 1.0f);
    Out.ViewPos = mul(WorldPos, gViewMatrix);
    Out.Pos = mul(Out.ViewPos, gProjMatrix);
    return Out;
}