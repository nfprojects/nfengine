cbuffer Global : register(b0)
{
    float4x4 g_CameraMatrix;
    float4x4 g_ViewMatrix;
    float4x4 g_ProjMatrix;
    float4x4 g_ProjInverseMatrix;
    float4x4 g_ViewProjMatrix;
    float4x4 g_ViewProjInverseMatrix;
    float4 g_ViewportResInv;
    float4 g_ScreenScale;
};

cbuffer SpotLightProps : register(b1)
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float4 FarDist;
    float4x4 LightViewProjMatrix;
};

struct VS_IN
{
    float3 Pos : POSITION;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 ViewPos : POSITION;
};

VS_OUTPUT main(VS_IN In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    //position transformation
    Out.ViewPos = mul(float4(In.Pos, 1.0f), g_ViewMatrix);
    Out.Pos = mul(Out.ViewPos, g_ProjMatrix);

    return Out;
}