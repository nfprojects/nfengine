#if (USE_TEXTURE > 0)
	Texture2D tex0 : register(t0);
	SamplerState tex_sampler : register(s0);
#endif


struct VS_OUTPUT
{
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR;
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	float4 color = In.Color;

#if (USE_TEXTURE > 0)

#if (TEXTURE_MODE == 0) // normal mode
	color *= tex0.Sample(tex_sampler, In.TexCoord);
#else // alpha mode
	color.a *= tex0.Sample(tex_sampler, In.TexCoord).a;
#endif

	//color.a *= tex0.Sample(tex_sampler, In.TexCoord).a;
#endif	
	
	return color;
}

