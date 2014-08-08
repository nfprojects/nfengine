//BLUR - PIXEL SHADER

Texture2D g_Source : register(t0);

cbuffer g_Params : register (b1)
{
	int4 g_InputRes;
	float4 g_Params;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//StructuredBuffer<float> g_Weights : register(t1);

//---------------------------------------------------------------

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	int3 coords;
	
	float3 color;
	float4 sum = float4(0, 0, 0, 0);

	int size = min(256, g_Params.x);
	
	#if (MODE == 0)	
		bool border = (texelCoords.x < size) || (texelCoords.x >= g_InputRes.x-size);
	#else
		bool border = (texelCoords.y < size) || (texelCoords.y >= g_InputRes.y-size);
	#endif
	
	[branch]
	if (border)
	{
		#if (MODE == 0)
			int size_min = max(0, texelCoords.x - size) - texelCoords.x;
			int size_max = min(g_InputRes.x-1, texelCoords.x + size) - texelCoords.x;
		#else
			int size_min = max(0, texelCoords.y - size) - texelCoords.y;
			int size_max = min(g_InputRes.y-1, texelCoords.y + size) - texelCoords.y;
		#endif

		[loop]
		for (int i = size_min; i <= size_max; i++)
		{

		#if (MODE == 0)
			coords = texelCoords + int3(i, 0, 0);
			//coords.x = clamp(coords.x, 0, g_InputRes.x-1);
		#else
			coords = texelCoords + int3(0, i, 0);
			//coords.y = clamp(coords.y, 0, g_InputRes.y-1);
		#endif
			
			float weight = exp(-(i*i) * g_Params.y);
			color = g_Source.Load(coords).rgb;
			sum += float4(color, 1) * weight;
		}

		//sum.xyz /= sum.w;
	}
	else
	{
		float weight; // = g_Weights[0];
		color = g_Source.Load(texelCoords).rgb;	
		sum = float4(color, 1);

		[loop]
		for (int i = 1; i <= size; i++)
		{
			//float weight = g_Weights[i];
			float weight = exp(-(i*i) * g_Params.y);

		
		#if (MODE == 0)
			coords = texelCoords + int3(i, 0, 0);
		#else
			coords = texelCoords + int3(0, i, 0);
		#endif		
			color = g_Source.Load(coords).rgb;
			sum += float4(color, 1) * weight;
			
			
		#if (MODE == 0)
			coords = texelCoords - int3(i, 0, 0);
		#else
			coords = texelCoords - int3(0, i, 0);
		#endif		
			color = g_Source.Load(coords).rgb;
			sum += float4(color, 1) * weight;			
		}	
	}

	/*
	[loop]
	for (int i = -size; i<=size; i++)
	{
		#if (MODE == 0)	
			coords = texelCoords + int3(i, 0, 0);
		#else
			coords = texelCoords + int3(0, i, 0);
		#endif	

		float weight = exp(-(i*i) * g_Params.y);

	#if (MODE == 0)	
		if ((coords.x < 0) || (coords.x >= g_InputRes.x))
			weight = 0;
	#else
		if ((coords.y < 0) || (coords.y >= g_InputRes.y))
			weight = 0;
	#endif

		sum += float4(g_Source.Load(coords).rgb, 1) * weight;
	}*/
	
	return float4(sum.xyz/sum.w, 1);
}

