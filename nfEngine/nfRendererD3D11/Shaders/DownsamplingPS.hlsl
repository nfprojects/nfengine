//DOWNSAPLING - PIXEL SHADER

Texture2D<float4> g_Source : register(t0);
SamplerState tex_sampler : register(s0);

cbuffer g_Params : register (b1)
{
	float4 g_BufferInvRes;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VS_OUTPUT In) : SV_TARGET0
{

	//use bilinear sampling to average 4 texels in one instruction
	float4 coords = 4.0 * In.Pos.xyxy + float4(0.0, 0.0, 2.0, 2.0);
	coords *= g_BufferInvRes.xyxy;
	
	float3 color;
	color = g_Source.SampleLevel(tex_sampler, coords.xy, 0);
	color += g_Source.SampleLevel(tex_sampler, coords.zy, 0);
	color += g_Source.SampleLevel(tex_sampler, coords.xw, 0);
	color += g_Source.SampleLevel(tex_sampler, coords.zw, 0);
	color /= 4.0f;

/*
	int3 texelCoords = 4 * int3((int2)In.Pos.xy, 0);
	float3 color = float3(0, 0, 0);
	
	[unroll] for (int i = 0; i<4; i++)
	{
		float3 rowColor = float3(0, 0, 0);
		[unroll] for (int j = 0; j<4; j++)
		{
			rowColor += g_Source.Load(texelCoords + int3(i, j, 0)).rgb;
		}
		color += rowColor;
	}		
	color /= 16.0f;
*/
	
	float gray = dot(color, float3(0.3f, 0.59f, 0.11f)); 
	float3 weight = 1.0f - exp(-0.33 * color); //bright pass
					
	return float4(weight * color, gray); //store gray color in alpha
}

