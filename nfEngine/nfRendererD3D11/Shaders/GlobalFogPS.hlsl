//AMBIENT LIGHT - PIXEL SHADER

//g-buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);
Texture2D<float> tex_depth : register(t4);


SamplerState g_PointSampler : register(s2);

cbuffer g_Params : register (b1)
{
	uint4 g_OutResolution;
	float4 g_AmbientLight;
	float4 g_BackgroundColor;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float depth = tex_depth.Load(texelCoords); //depth
	depth *= 1000.0f;
	//float3 normal = tex0.Load(texelCoords).xyz; //depth
	
	float blendFactor = exp(- depth * g_BackgroundColor.w);
		
	return float4(g_BackgroundColor.xyz, blendFactor);
}