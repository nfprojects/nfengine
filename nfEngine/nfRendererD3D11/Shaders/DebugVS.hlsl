cbuffer Global : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 ProjMatrix;
}

struct VS_IN
{
	float3 Pos : POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD0;
	float4 Pos : SV_POSITION;
};


//---------------------------------------------------------------

VS_OUTPUT main(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	Out.Pos = mul(float4(In.Pos, 1.0f), ViewMatrix);
	Out.Pos = mul(Out.Pos, ProjMatrix);
	
	Out.Color = In.Color;
	Out.TexCoord = In.TexCoord;
	
	return Out;
}


