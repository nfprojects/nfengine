//AMBIENT LIGHT - COMPUTE SHADER

//G-Buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);

//render target
RWTexture2D<float4> g_OutBuff : register(u0);

cbuffer g_Params : register (b0)
{
	uint4 g_OutResolution;
	float4 g_AmbientLight;
	float4 g_BackgroundColor;
};

#define THREADS_X (32)
#define THREADS_Y (32)

[numthreads(THREADS_X, THREADS_Y, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID,
		  uint3 groupID : SV_GroupID,
		  uint3 dispatchThreadID : SV_DispatchThreadID,
		  uint groupIndex : SV_GroupIndex)
{
	//check if pixel is inside render target
	if (any(dispatchThreadID.xy >= g_OutResolution.xy))
		return;

	
	int3 texelCoords = int3(dispatchThreadID.xy, 0);
	float4 worldPos = tex0.Load(texelCoords); //world pos
	float4 diffuseColor = tex2.Load(texelCoords); //color

	float4 result;
	if (worldPos.x < 1e+9)
		result = diffuseColor * g_AmbientLight;
	else
		result = g_BackgroundColor;

	//TODO: Ambient Occlusion
		
	g_OutBuff[dispatchThreadID.xy] = float4(result.xyz, 0.0f);
}