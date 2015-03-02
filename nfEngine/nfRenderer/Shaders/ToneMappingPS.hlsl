//TONE MAPPING - PIXEL SHADER

Texture2D<float4> g_Source : register(t0);
Texture2D<float4> g_Bloom : register(t1);
Texture2D g_Noise : register(t2);

SamplerState g_ClampingSampler : register(s0);
SamplerState tex_sampler : register(s1);

cbuffer g_Params : register (b1)
{
	float4 g_BufferInvRes;
	float4 g_OutResolution;
	float4 g_Params; //exposure, bloom factor, noise factor, saturation
	float4 g_NoiseSeed;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------


float rand_1_05(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv ,float2(12.9898,78.233)*2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}

float2 rand_2_10(in float2 uv) {
    float noiseX = (frac(sin(dot(uv, float2(12.9898,78.233) * 2.0)) * 43758.5453));
    float noiseY = sqrt(1 - noiseX * noiseX);
    return float2(noiseX, noiseY);
}

float2 rand_2_0004(in float2 uv)
{
    float noiseX = (frac(sin(dot(uv, float2(12.9898,78.233)      )) * 43758.5453));
    float noiseY = (frac(sin(dot(uv, float2(12.9898,78.233) * 2.0)) * 43758.5453));
    return float2(noiseX, noiseY) * 0.004;
}

float randf(inout uint seed)
{
	seed = 1103515245*seed + 12345;
    return asfloat((seed >> 9) | 0x3f800000) - 1.0f;
}

float3 FilmicToneMapping(float3 color, float exposure)
{
	color *= exposure;
	float3 x = max(0,color-0.004);
	color = (x*(6.2*x+0.5))/(x*(6.2*x+1.7)+0.06);
	color = pow(color, 2.2);
	return color;
}

float3 ToneMapping(float3 color, float exposure)
{
	return float3(1, 1, 1) - exp(-color*exposure);
}

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float2 texelCoordsF = In.Pos.xy * g_BufferInvRes.xy;
	float3 color = g_Source.Load(texelCoords).rgb;

#if (BLOOM > 0)
	
	float3 bloomColor = g_Bloom.SampleLevel(g_ClampingSampler, texelCoordsF, 0).rgb;	
	color += g_Params.y * bloomColor;
#endif	
	
	//saturation
	float gray = dot(color, float3(0.3f, 0.59f, 0.11f));
	color = lerp(gray.xxx, color, g_Params.w);
	
	//tone mapping
	float exposure = g_Params.x;
	float3 result = FilmicToneMapping(color, exposure);

	//gamma correction
#if (GAMMA_CORRECTION > 0)	
	result = sqrt(result);
#endif	
	
#if (NOISE > 0)	
	float seed =  rand_1_05(texelCoordsF.xy + 2.0 * g_NoiseSeed.y);
	result += (seed-0.5).xxx * g_Params.z;
#endif	

	return float4(result, 1.0f);
}

