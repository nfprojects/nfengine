Texture2D<float4> g_Source : register(t0);
Texture2D g_Noise : register(t1);

SamplerState g_ClampingSampler : register(s0);
SamplerState tex_sampler : register(s1);

cbuffer g_Params : register (b1)
{
	float4 g_BufferInvRes;
	float4 g_OutResolution;
	float4 g_Params; //noise level, span_max, 1 / reduce_mul, 1 / reduce_min
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

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float2 texelCoordsF = In.Pos.xy * g_BufferInvRes.xy;
	float3 result = g_Source.Load(texelCoords).rgb;

	

#if (ANTIALIASING > 0)
	#define INV (g_BufferInvRes.xy)
	#define Sample(coords) g_Source.SampleLevel(g_ClampingSampler, (coords), 0)
	#define Samoff(XX,YY) Sample(texelCoordsF+(float2((XX),(YY))*INV)).rgb;
	#define Samoff2(XX,YY) g_Source.Load(texelCoords+(int3((XX),(YY), 0))).rgb;

	#if (LUMA_OPT > 0)
		#define luma(c) (c.y * (0.587/0.299) + c.x)
	#else
		#define luma(c) (dot(c, float3(0.299, 0.587, 0.114)))
	#endif
	
	const float FXAA_SPAN_MAX = g_Params.y; //8.0;
	const float FXAA_REDUCE_MUL = g_Params.z; //1.0/8.0;
	const float FXAA_REDUCE_MIN = g_Params.w; //1.0/128.0;

	float3 rgbNW = Samoff2(-1.0,-1.0);
	float3 rgbNE = Samoff2(1.0,-1.0);
	float3 rgbSW = Samoff2(-1.0,1.0);
	float3 rgbSE = Samoff2(1.0,1.0);
	float3 rgbM	 = result;
	
	float	lumaNW = luma(rgbNW), lumaNE = luma(rgbNE), 
			lumaSW = luma(rgbSW), lumaSE = luma(rgbSE), lumaM  = luma(rgbM);
	float	lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float	lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	float2	dir = float2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)), ((lumaNW + lumaSW) - (lumaNE + lumaSE)));
	float	dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	float	rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
			dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * INV;
	float3	rgbA = (1.0/2.0) * (Sample(texelCoordsF + dir * (1.0/3.0 - 0.5)).rgb + Sample(texelCoordsF + dir * (2.0/3.0 - 0.5)).rgb);
	float3	rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (Sample(texelCoordsF + dir * (0.0/3.0 - 0.5)).rgb + Sample(texelCoordsF + dir * (3.0/3.0 - 0.5)).rgb);
	float	lumaB = luma(rgbB);

	result = ((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB;
#endif

	#define luma_rg(c) (c.y * (0.587/0.299) + c.x)
	#define luma_rgb(c) (dot(c, float3(0.299, 0.587, 0.114)))

	
	//result.rg = pow(result.rg, 0.5);
	//result = lerp(float3(1,1,0), pow(result, 0.7), fade);
	//result = lerp(float3(0,0,1), pow(result, 0.7), luma_f(result));

	/*
	//filmic filter
	float fad = 1-pow(length(texelCoordsF - 0.5 * g_OutResolution.xy * g_BufferInvRes.xy)/0.4, 2);
	float2 lum = float2(luma_rg(result), luma_rgb(result));
	float3 hot = result * float3(1.5, 1.2, 0.9);
	float3 cold = result * float3(0.7, 1.0, 1.4);
	float3 bord = result * float3(0.2 + lum.x*0.5, 0.2 + lum.x*0.4, 0.1);
	result = lerp(result, hot, lum.x);
	result = lerp(result, cold, 1 - lum.y);
	result = lerp(bord, result, fad);
	*/

	//result = float3(1,1,1) - exp(-2*result);
	//dithering
#if (NOISE > 0)	
	float seed =  rand_1_05(texelCoordsF.xy + 2.0 * g_NoiseSeed.y);
	result += (seed-0.5).xxx * g_Params.r;
#endif	
	
	

	return float4(result, 1.0f);
}
