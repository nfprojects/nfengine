//AMBIENT LIGHT - PIXEL SHADER

//g-buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);
Texture2D<float> tex_depth : register(t4);

Texture2DArray<float> g_CascadedShadowMap : register(t8);

SamplerComparisonState g_ShadowSampler : register(s0);
SamplerState g_LinSampler : register(s1);
SamplerState g_PointSampler : register(s2);

cbuffer Global : register(b0)
{
	float4x4 g_CameraMatrix;
	float4x4 g_ViewMatrix;
	float4x4 g_ProjMatrix;
	float4x4 g_ProjInverseMatrix;
	float4x4 g_ViewProjMatrix;
	float4x4 g_ViewProjInverseMatrix;
	float4 g_ViewportResInv;
	float4 g_ScreenScale;
};

cbuffer g_Params : register (b1)
{
	float4 g_LightDir;
	float4 g_LightColor;
	uint4 g_CascadesCount;
	
	float4 g_CascadeRanges[8];
	row_major float4x4 g_LightViewProjMatrix[8];
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

float3 TransformLightCoords(float4 In)
{
	float3 result = In.xyz / In.w;
	result.x = result.x * 0.5 + 0.5;
	result.y = -result.y * 0.5 + 0.5;
	return result;
}

float SampleShadow(float2 coords, float z0, int level) {
	if (any(coords < float2(0, 0))) return 1.0f;
	if (any(coords > float2(1, 1))) return 1.0f;
	if (z0 > 1) return 1;
	return g_CascadedShadowMap.SampleCmpLevelZero(g_ShadowSampler, float3(coords, (float)level), z0);
}

float SampleShadowPCF(float2 coords, float z0, int level)
{
	if (any(coords < float2(0, 0))) return 1.0f;
	if (any(coords > float2(1, 1))) return 1.0f;
	if (z0 > 1) return 1;

	float ShadowValue = 0.0f;
	const int PCF_size = 1;
	[unroll] for (int i = -PCF_size; i<=PCF_size; i++)
	{
		[unroll] for (int j = -PCF_size; j<=PCF_size; j++)
		{
			float2 offset = float2(i, j) / 1024.0f; //shadow map resolution
			ShadowValue += g_CascadedShadowMap.SampleCmpLevelZero(g_ShadowSampler, float3(coords+offset, (float)level), z0);
		}
	}
	return ShadowValue / ((2*PCF_size+1)*(2*PCF_size+1));

	//return g_CascadedShadowMap.SampleCmpLevelZero(g_ShadowSampler, float3(coords, (float)level), z0);
}

float ComputeShadow(float3 Pos, uint i) {
	float3 LightSpacePos = TransformLightCoords(mul(float4(Pos, 1), g_LightViewProjMatrix[i]));	
	return SampleShadow(LightSpacePos.xy, LightSpacePos.z, i);
}

float ComputeShadowCascade(float3 Pos, float depth, uint startCascade = 0) {
	float ShadowValue = 1;
	for (uint i = startCascade; i < g_CascadesCount.x; i++)
	{
		float frustum_far = g_CascadeRanges[i].y;

		if (depth < frustum_far)
		{
			float3 LightSpacePos = TransformLightCoords(mul(float4(Pos, 1), g_LightViewProjMatrix[i]));	
			ShadowValue = SampleShadowPCF(LightSpacePos.xy, LightSpacePos.z, i);

			//blend
			if (i < g_CascadesCount.x - 1)
			{
				float frustum_near = g_CascadeRanges[i+1].x;
				if (depth > frustum_near)
				{
					float3 LightSpacePosB = TransformLightCoords(mul(float4(Pos, 1), g_LightViewProjMatrix[i+1]));	
					float ShadowValueB = SampleShadowPCF(LightSpacePosB.xy, LightSpacePosB.z, i+1);
					
					float blendFactor = (depth - frustum_near) / (frustum_far - frustum_near);
					ShadowValue = lerp(ShadowValue, ShadowValueB, blendFactor);
				}
			}
			
			break;
		}
	}
	return ShadowValue;
}

float ComputeShadowCascadeSelect(float3 Pos, float depth) {
	int selected = 0;
	[unroll] for (int i = 3; i >= 0; i--) {
		float frustum_far = g_CascadeRanges[i].y;
		if(depth < frustum_far) {
			selected = i;
		}
	}
	return ComputeShadow(Pos, selected);
}

float ComputeShattering(float3 Pos, float3 Dir, float max_depth) 
{
//mozna do cbuffera przeniesc i sparametryzowac w gui/config
const uint	depth_start = 0.1;
const uint	depth_limit = 64;
const uint	step_num	= 32;
const float shatter_int	= 0.09;
const float shatter_lum	= 0.88;
const float rel_step	= 0.08;

	const float	step		= depth_limit / (float) step_num;
	const float	sum_norm	= 1. / (float) step_num;
	float		shadow		= 0;
	float		depth		= depth_start;
	
	max_depth = min(max_depth, depth_limit);

	[unroll] for(uint i = 0; i < step_num ; i++)
	{
		shadow	+= ComputeShadowCascadeSelect(Pos + Dir * depth, depth);
		depth	+= min(step, (max_depth - depth)*rel_step);
	}

	return shatter_int * pow(shadow, shatter_lum) * sum_norm * g_LightColor.xyz;
}

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float3 result = 0;
	float depth = clamp(tex_depth.Load(texelCoords), 0, 1); //depth
	depth *= 1000.0f;

	float2 projectedXY = In.Pos.xy * g_ViewportResInv.xy * 2.0 - float2(1.0, 1.0); //scale to [-1 .. 1]
	projectedXY.y = -projectedXY.y;

	float3 viewSpaceRay = g_CameraMatrix[2].xyz + 
						  g_ScreenScale.x * projectedXY.x * g_CameraMatrix[0].xyz +
						  g_ScreenScale.y * projectedXY.y * g_CameraMatrix[1].xyz; 

	float3 CameraPosition = g_CameraMatrix[3].xyz;
	float3 Pos = CameraPosition + viewSpaceRay * depth;

	if(depth < 1000) 
	{
		float4 color0 = tex0.Load(texelCoords); //normal
		float4 color1 = tex1.Load(texelCoords); //color
		//float4 color2 = tex2.Load(texelCoords); //<unused>
		//float4 color3 = tex3.Load(texelCoords); //<unused>

		float3 Normal = color0.xyz;
		float3 Color = color1.xyz;
		float SpecularFactor = color0.w;
		float SpecularPower = color1.w;

		float NdotL = saturate(dot(Normal, -g_LightDir.xyz));
		if(NdotL >= 0) 
		{
			float ShadowValue = ComputeShadowCascade(Pos, depth);
		
			//specular
			float3 Specular = float3(0, 0, 0);
			float3 EyeVector = normalize(viewSpaceRay);
			float3 ReflectVector = reflect(EyeVector, Normal);
			float RdotL = dot(ReflectVector, -g_LightDir.xyz);
			if (RdotL > 0)
			{
				Specular = SpecularFactor * pow(RdotL, SpecularPower);
			}	
	
			result = g_LightColor.xyz * ShadowValue * (Color * NdotL + Specular);
		}
	}

	//result += ComputeShattering(CameraPosition, (viewSpaceRay), depth);
	
	return float4(result, 1.0f);	
}









