cbuffer Global : register(b0)
{
	float4x4 ProjMatrix;
}

struct VS_INPUT_OUTPUT
{
	int4 Rect    : POSITION;
	float4 TexCoords : TEXCOORD0;
	float4 Color : COLOR0;
};


//---------------------------------------------------------------

VS_INPUT_OUTPUT main(VS_INPUT_OUTPUT In)
{
	return In;
}


