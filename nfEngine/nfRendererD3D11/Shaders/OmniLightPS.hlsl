//OMNI LIGHT PIXEL SHADER

//g-buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);
Texture2D<float> tex_depth : register(t4);

SamplerComparisonState g_ShadowSampler : register(s0);
SamplerState g_LightMapSampler : register(s1);

#if (USE_SHADOWS > 0)
	TextureCube<float> g_ShadowMap : register(t8);
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

cbuffer OmniLightProps : register(b1)
{
	float4 LightPos;
	float4 LightRadius;
	float4 LightColor;
	float4 ShadowMapResInv;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 ViewPos : POSITION;
};

//---------------------------------------------------------------

struct PS_OUTPUT
{
	float depth : SV_Depth;
	float4 color : SV_TARGET0;
};

PS_OUTPUT main(VS_OUTPUT In)
{
	PS_OUTPUT output;
	output.depth = In.ViewPos.z * 0.001f;

	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float4 color0 = tex0.Load(texelCoords); //normal
	float4 color1 = tex1.Load(texelCoords); //color
	//float4 color2 = tex2.Load(texelCoords); //<unused>
	//float4 color3 = tex3.Load(texelCoords); //<unused>

	float depth = tex_depth.Load(texelCoords); //depth
	clip(0.99999f - depth);
	depth *= 1000.0f;

	//depth = 0.05f + (1000.0 - 0.05f) * depth;

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
	
	float3 LightVec = LightPos.xyz - Pos;
	float LightDist = length(LightVec);
	LightVec /= LightDist; //normalize
	
	float NdotL = dot(Normal, LightVec);
	clip(NdotL);	//if (NdotL<0)	return;
	clip(LightRadius.x - LightDist);	//if (LightDist > radius)	return;


#if (USE_SHADOWS > 0)

	float3 OffsetU = float3(1, 0, 0);
	float3 OffsetV = float3(0, 0, 1);
	
	if ((LightVec.x > sqrt(2.0)/2.0) || (LightVec.x < -sqrt(2.0)/2.0))
	{
		OffsetU = float3(0, 0, 1);
		OffsetV = float3(0, 1, 0);
	}
	else
	if ((LightVec.z > sqrt(2.0)/2.0) || (LightVec.z < -sqrt(2.0)/2.0))
	{
		OffsetU = float3(1, 0, 0);
		OffsetV = float3(0, 1, 0);
	}
	
	const int PCF_Size = 1;
	float ShadowValue = 0.0;
	[unroll] for (int x = -PCF_Size; x<=PCF_Size; x++)
		[unroll] for (int y = -PCF_Size; y<=PCF_Size; y++)
			ShadowValue += g_ShadowMap.SampleCmpLevelZero(g_ShadowSampler, -LightVec + ShadowMapResInv.x * (x*OffsetU + y*OffsetV), LightDist).x;
	ShadowValue /= (float)((2*PCF_Size+1)*(2*PCF_Size+1));	
	
	//float ShadowValue = g_ShadowMap.SampleCmpLevelZero(g_ShadowSampler, -LightVec, LightDist); //no PCF
#else	
	float ShadowValue = 1;
#endif


	float fadeOut = 1.0f - LightDist/LightRadius.x;
	fadeOut *= fadeOut;
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

	output.color = float4(LightColor.xyz * fadeOut * ShadowValue * (Color * NdotL + Specular), 1.0f);
	return output;



	/*
	// LINE LIGHT SOURCE EXPERIMENTS

	// P1 - LightPos.xyz
	float3 D = float3(-10, -0.0f, 0);
	float radius = 5.0f;

	float j = dot(Normal, LightPos.xyz - Pos);
	float k = dot(Normal, D);
	float l = dot(D, D);
	float m = dot(LightPos.xyz - Pos, D);
	float n = dot(LightPos.xyz - Pos, LightPos.xyz - Pos);


	float line_dist = n - m*m/l;
	clip(radius - line_dist);

	float d_max = dot(Pos - LightPos.xyz, Normal) / k;

	float2 x = float2(0, 1);

	if (k < -0.00001)
		x = min(x, d_max - 0.1);
	else if (k > 0.00001)
		x = max(x, d_max + 0.1);

	float epsilon = 0.00001f;
	float2 I = (k*(n+m*x) - j*(m+l*x)) / ((m*m-l*n + epsilon) * sqrt(epsilon+n+x*(2*m+l*x)));	//integral (j+k*x)/(l*x^2+2*m*x+n)^(3/2) dx
	float factor = max(0, I.y - I.x);

	output.color = float4(Color * LightColor.xyz * factor, 1.0f);
	return output;
	*/
}

