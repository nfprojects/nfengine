//SPOT LIGHT PIXEL SHADER

//g-buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);
Texture2D<float> tex_depth : register(t4);

SamplerComparisonState g_ShadowSampler : register(s0);
SamplerState g_LightMapSampler : register(s1);

#if (USE_SHADOWS > 0)
	Texture2D<float2> g_ShadowMap : register(t8);
#endif

#if (USE_LIGHT_MAP > 0)
	Texture2D<float3> g_LightMap : register(t9);
#endif

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

cbuffer SpotLightProps : register(b1)
{
	float4 LightPos;
	float4 Direction;
	float4 LightColor;
	float4 FarDist;
	row_major float4x4 g_LightViewProjMatrix;
	row_major float4x4 g_LightViewMatrix;
	float ShadowMapResInv;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 ViewPos : POSITION;
};

//---------------------------------------------------------------

float3 TransformLightCoords(float4 In)
{
	float3 result = In.xyz / In.w;
	
	result.x = result.x * 0.5 + 0.5;
	result.y = -result.y * 0.5 + 0.5;
	return result;
}

struct PS_OUTPUT
{
	float depth : SV_Depth;
	float4 color : SV_TARGET0;
};
	
PS_OUTPUT main(VS_OUTPUT In)
{
	PS_OUTPUT output;
	output.depth = In.ViewPos.z;

	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float4 color0 = tex0.Load(texelCoords); //normal
	float4 color1 = tex1.Load(texelCoords); //color
	//float4 color2 = tex2.Load(texelCoords); //<unused>
	//float4 color3 = tex3.Load(texelCoords); //<unused>

	float depth = tex_depth.Load(texelCoords); //depth
	clip(0.99999f - depth);
	depth *= 1000.0f;

	float2 projectedXY = In.Pos.xy * g_ViewportResInv.xy * 2.0 - float2(1.0, 1.0); //scale to [-1 .. 1]
	projectedXY.y = -projectedXY.y;

	float3 viewSpaceRay = g_CameraMatrix[2].xyz + 
						  g_ScreenScale.x * projectedXY.x * g_CameraMatrix[0].xyz +
						  g_ScreenScale.y * projectedXY.y * g_CameraMatrix[1].xyz;

	float3 CameraPosition = g_CameraMatrix[3].xyz;
	float3 Pos = CameraPosition + viewSpaceRay * depth;
	float3 Normal = color0.xyz;
	float3 Color = color1.xyz;
	float SpecularFactor = color0.w;
	float SpecularPower = color1.w;
	
	float3 LightSpacePos = TransformLightCoords(mul(float4(Pos, 1), g_LightViewProjMatrix));
	
	
	float3 termA = LightSpacePos > float3(0, 0, 0);
	float3 termB = LightSpacePos < float3(1, 1, 1);
	clip(dot(termA, termB) - 3.0f);
	

	float3 LightVec = LightPos.xyz - Pos;
	float LightDist = length(LightVec);
	LightVec /= LightDist; //normalize
	
	float NdotL = dot(Normal, LightVec);
	NdotL = max(NdotL, 0);
	clip(NdotL);
	clip(FarDist.x - LightDist);
	
#if (USE_LIGHT_MAP > 0)	
	float3 LightMapColor = g_LightMap.SampleLevel(g_LightMapSampler, LightSpacePos.xy, 0);
	clip(dot(LightMapColor, float3(1, 1, 1)) - 0.00001f);
#endif
	
	
	float ShadowValue = 1;
	
#if (USE_SHADOWS > 0)


	ShadowValue = 0.0f;
	const int PCF_size = 1;
	for (int i = -PCF_size; i<=PCF_size; i++)
	{
		for (int j = -PCF_size; j<=PCF_size; j++)
		{
			float2 offset = float2(i, j) * ShadowMapResInv; //shadow map resolution
			ShadowValue += g_ShadowMap.SampleCmpLevelZero(g_ShadowSampler, LightSpacePos.xy+offset, LightSpacePos.z);
		}
	}
	ShadowValue /= (2*PCF_size+1)*(2*PCF_size+1);
	clip(ShadowValue - 0.00001f);

#endif

	
	float fadeOut = 1.0f - LightDist/FarDist.x;
	fadeOut /= (LightDist*LightDist);
	
	//specular
	float3 Specular = float3(0, 0, 0);
	float3 EyeVector = normalize(viewSpaceRay);
	float3 ReflectVector = reflect(EyeVector, Normal);
	float RdotL = dot(ReflectVector, LightVec);
	if (RdotL > 0)
	{
		Specular = SpecularFactor * pow(RdotL, SpecularPower);
	}
	
	float3 result = LightColor.xyz * ShadowValue * fadeOut * (Color * NdotL + Specular);
	
	
#if (USE_LIGHT_MAP > 0)
	result *= LightMapColor;
#endif
	
	output.color = float4(result, 1);
	return output;
}

