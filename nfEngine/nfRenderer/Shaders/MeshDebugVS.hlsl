cbuffer Global : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 ProjMatrix;
}

struct VS_IN
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD0;
	
};


//---------------------------------------------------------------

VS_OUTPUT main(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	
	float4 eyeSpacePos = mul(float4(In.Pos, 1.0f), ViewMatrix);
	Out.Pos = mul(eyeSpacePos, ProjMatrix);
	Out.TexCoord = In.TexCoord;
	
	return Out;
}


