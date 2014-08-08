//AVERAGING - COMPUTE SHADER

//Input texture
Texture2D<float4> tex0 : register(t0);

//output buffer
RWStructuredBuffer<float> g_OutBuff : register(u0);

cbuffer g_Params : register (b0)
{
	uint4 g_OutResolution;
};

[numthreads(1, 1, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID,
		  uint3 groupID : SV_GroupID,
		  uint3 dispatchThreadID : SV_DispatchThreadID,
		  uint groupIndex : SV_GroupIndex)
{
	//check if pixel is inside render target
	if (dispatchThreadID.y >= g_OutResolution.y)
	{
		g_OutBuff[dispatchThreadID.y] = 0;
		return;
	}

	uint width = min(2048, g_OutResolution.x);
		
	//average texels in one row
	float color = 0;
	for (uint i = 0; i<width; i++)
	{
		color += tex0.Load(int3(i, dispatchThreadID.y, 0)).a;
	}

	g_OutBuff[dispatchThreadID.y] = color;
}