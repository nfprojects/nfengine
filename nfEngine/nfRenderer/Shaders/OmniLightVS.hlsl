
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

cbuffer OmniLightProps : register(b1)
{
	float4 Position;
	float4 Radius;
	float4 Color;
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


//---------------------------------------------------------------

VS_OUTPUT main(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	
	//position transformation
	float4 WorldPos = float4(Position.xyz + In.Pos*Radius.x, 1.0f);
	Out.ViewPos = mul(WorldPos, g_ViewMatrix);
	Out.Pos = mul(Out.ViewPos, g_ProjMatrix);
	
	return Out;
}


