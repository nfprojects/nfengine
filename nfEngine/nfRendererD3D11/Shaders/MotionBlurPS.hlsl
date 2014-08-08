//MOTION BLUR - PIXEL SHADER

Texture2D<float4> g_Source : register(t0);
Texture2D<float2> g_MotionBuffer : register(t1);
SamplerState g_TexSampler : register(s1);
SamplerState g_TexSamplerAniso : register(s3);

cbuffer g_Params : register (b1)
{
	float4 g_BufferInvRes;
	float4 g_OutResolution;
	float4 g_Factor;
	uint4 g_Seed;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	float4 bufferBorders = float4(0, 0, -g_OutResolution.xy) * g_BufferInvRes.xyxy;

	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float2 coords = In.Pos.xy * g_BufferInvRes.xy;
	float2 velocity_vec = g_Factor.x * g_MotionBuffer.Load(texelCoords) * 0.5f;


	// TODO: move to cbuffer
	const float minSamples = 1; // the lower, the more at low movement speeds
	const float maxSamples = 24;

	float samplesCount = max(1, min(maxSamples, length(velocity_vec * g_OutResolution.xy)));
	float samplesCountInv = 1.0f / samplesCount;

	float4 color = 0;
	for (float i = 0.0f; i<samplesCount-0.5; i += 1.0f)
	{
		float2 pos = coords + velocity_vec * (i*samplesCountInv - 0.5);

		//check if pixel is within output rectangle
		bool4 cond = float4(pos, -pos) > bufferBorders;
		float weight = all(cond);

		color += weight * float4(g_Source.SampleLevel(g_TexSampler, pos, 0).rgb, 1);	

		// HQ mode
		//color += weight * float4(g_Source.SampleGrad(g_TexSamplerAniso, pos, velocity_vec * 0.5f * samplesCountInv, float2(0, 0)).rgb, 1);
	}

	float3 finalColor;

	if (color.w > 0)
		finalColor = color.xyz / color.w;
	else
		finalColor = g_Source.Load(texelCoords); 

	return float4(finalColor, 1.0f);
}

