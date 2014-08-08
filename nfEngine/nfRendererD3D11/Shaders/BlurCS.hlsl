//BLUR - COMPUTE SHADER

Texture2D<float4> g_Source : register(t0); //source
RWTexture2D<float4> g_OutBuff : register(u0); //destination

cbuffer g_Params : register (b1)
{
	int4 g_InputRes;
	float4 g_Params;
};

//max size of processed texture
#define MAX_SIZE (512)

groupshared float3 s_Line[MAX_SIZE];


#if (MODE == 0)
	[numthreads(MAX_SIZE, 1, 1)]
#else
	[numthreads(1, MAX_SIZE, 1)]
#endif


//[numthreads(32, 32, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID,
		  uint3 groupID : SV_GroupID,
		  uint3 dispatchThreadID : SV_DispatchThreadID,
		  uint groupIndex : SV_GroupIndex)
{
	//load line to the TLS
	#if (MODE == 0)	
		s_Line[dispatchThreadID.x] = g_Source.Load(int3(dispatchThreadID.xy, 0)).xyz;
	#else
		s_Line[dispatchThreadID.y] = g_Source.Load(int3(dispatchThreadID.xy, 0)).xyz;
	#endif

	// sync
	GroupMemoryBarrierWithGroupSync();
	
	if (any(dispatchThreadID.xy >= g_InputRes.xy))
		return;

	int coords;
	int size = min(256, g_Params.x);
	float4 sum = 0; // xyz = color, w = weight sum


	[loop]
	for (int i = -size; i<=size; i++)
	{
		#if (MODE == 0)	
			coords = (int)dispatchThreadID.x + i;
		#else
			coords = (int)dispatchThreadID.y + i;
		#endif	

		float weight = exp(-(i*i) * g_Params.y);

	#if (MODE == 0)	
		if ((coords < 0) || (coords >= g_InputRes.x))
			weight = 0;
	#else
		if ((coords < 0) || (coords >= g_InputRes.y))
			weight = 0;
	#endif

		sum += float4(s_Line[coords], 1) * weight;
	}	


	g_OutBuff[dispatchThreadID.xy] = float4(sum.xyz / sum.w, 1);
}




