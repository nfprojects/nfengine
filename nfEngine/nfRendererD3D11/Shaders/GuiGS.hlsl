cbuffer cb0
{
    row_major float4x4 g_ProjMatrix : packoffset(c0);
};


struct VS_OUT
{
	int4 Rect    : POSITION;
	float4 TexCoords : TEXCOORD0;
	float4 Color : COLOR0;
};
 
struct GS_OUT
{
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR;
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

[maxvertexcount(4)]
void main(point VS_OUT input[1], inout TriangleStream<GS_OUT> SpriteStream)
{
    GS_OUT Output;
	Output.Color = input[0].Color;
	
	float2 TmpPos;
	float4 Rect = float4(input[0].Rect);
	
	//Vertex 0
	Output.Pos = mul(float4(Rect.xz, 0.0, 1.0), g_ProjMatrix);
	Output.TexCoord.xy = input[0].TexCoords.xz;
	SpriteStream.Append(Output);
	
	//Vertex 1
	Output.Pos = mul(float4(Rect.yz, 0.0, 1.0), g_ProjMatrix);
	Output.TexCoord.xy = input[0].TexCoords.yz;
	SpriteStream.Append(Output);
	
	//Vertex 3
	Output.Pos = mul(float4(Rect.xw, 0.0, 1.0), g_ProjMatrix);
	Output.TexCoord.xy = input[0].TexCoords.xw;
	SpriteStream.Append(Output);
	
	//Vertex 2
	Output.Pos = mul(float4(Rect.yw, 0.0, 1.0), g_ProjMatrix);
	Output.TexCoord.xy = input[0].TexCoords.yw;
	SpriteStream.Append(Output);
	

    SpriteStream.RestartStrip();
}
