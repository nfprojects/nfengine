Texture2D tex0 : register(t0);
SamplerState tex_sampler : register(s0);

cbuffer Global : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 ProjMatrix;
	float4x4 ViewProjMatrix;
	float4 LightPos;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 ScreenPos : TEXCOORD1; //compiler bug???
};

//---------------------------------------------------------------

float main(VS_OUTPUT In) : SV_TARGET0
{
#if (USE_TEXTURE > 0)
	float4 color = tex0.Sample(tex_sampler, In.TexCoord);
	clip(color.a - 0.5);
#endif	
	
#if (WRITE_DISTANCE > 0)
	// for cube shadow mapping only
	float Depth = length(In.WorldPos - LightPos.xyz);
	float dx = ddx(Depth);  
	float dy = ddy(Depth);  
	return Depth + 0.0001f + 2*sqrt(dx*dx + dy*dy);
#else
	float Depth = In.ScreenPos.z / In.ScreenPos.w;
	
	float dx = ddx(Depth);  
	float dy = ddy(Depth);  
	return Depth + 0.0001f + 2.0f * sqrt(dx*dx + dy*dy);
#endif

}



