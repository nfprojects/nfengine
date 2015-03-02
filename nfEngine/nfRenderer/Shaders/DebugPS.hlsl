#if (USE_TEXTURE > 0)
	Texture2D tex0 : register(t0);
	SamplerState tex_sampler : register(s0);
#endif


struct VS_OUTPUT
{
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD0;
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	float4 color = In.Color;

#if (USE_TEXTURE > 0)
	color *= tex0.Sample(tex_sampler, In.TexCoord);
#endif	
	
	return color;
}

