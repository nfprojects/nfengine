cbuffer Global : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 ProjMatrix;
	float4x4 ViewProjMatrix;
	float4 LightPos;
};

cbuffer PerInstance : register(b1)
{
	float4x4 g_WorldMatrix;
};

struct VS_IN
{
	float3 Pos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
	float4 Tangent : TANGENT;
	
	
#if (INSTANCING > 0)
	float4 WordMat0 : TEXCOORD1;
	float4 WordMat1 : TEXCOORD2;
	float4 WordMat2 : TEXCOORD3;
	float4 Color : TEXCOORD5;
#endif	
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;	
	float4 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 ScreenPos : TEXCOORD1;
};

//---------------------------------------------------------------

VS_OUTPUT main(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	
#if (INSTANCING > 0)	
	float4x4 worldMatrix = transpose(float4x4(In.WordMat0, In.WordMat1, In.WordMat2, float4(0, 0, 0, 1)));
#else
	float4x4 worldMatrix = g_WorldMatrix;
#endif

	//position transformation
	Out.WorldPos = mul(float4(In.Pos, 1.0f), worldMatrix);
	Out.Pos = mul(Out.WorldPos, ViewProjMatrix);
	Out.ScreenPos = Out.Pos;
	Out.TexCoord = In.TexCoord;
	
	return Out;
}


